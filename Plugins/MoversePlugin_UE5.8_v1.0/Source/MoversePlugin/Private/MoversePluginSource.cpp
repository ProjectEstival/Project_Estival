// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MoversePluginSource.h"

#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Async/Async.h"
#include "Common/UdpSocketBuilder.h"
#include "Editor.h"
#include "Engine/SkeletalMesh.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "HAL/RunnableThread.h"
#include "ILiveLinkClient.h"
#include "Interfaces/IPluginManager.h"
#include "Json.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "Roles/LiveLinkLightRole.h"
#include "Roles/LiveLinkLightTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

DEFINE_LOG_CATEGORY(ModuleLog)

#define LOCTEXT_NAMESPACE "MoversePluginSourceEditorSource"

#define RECV_BUFFER_SIZE 1024 * 1024

FMoversePluginSource::FMoversePluginSource(FIPv4Endpoint InEndpoint,
    FString InActorName,
    bool InImportAssets)
    : ActorName(InActorName),
    Socket(nullptr),
    Stopping(false),
    Thread(nullptr),
    ImportAssets(InImportAssets),
    WaitTime(FTimespan::FromMilliseconds(100)) {
    // defaults
    if (InImportAssets && !LoadAndConnectAssets()) {
        return;
    }

    DeviceEndpoint = InEndpoint;

    FString MyStringPrintf = FString::Printf(TEXT("MyStringPrintf : {0}"));
    const FString MyStringFormatted =
        FString::Format(*MyStringPrintf, { ActorName });

    SourceStatus = LOCTEXT("SourceStatus_DeviceNotFound", "Device Not Found");
    SourceType =
        FText::Format(LOCTEXT("JSONLiveLinkSourceType", "Moverse Plugin ({0})"),
            FText::FromString(ActorName));
    SourceMachineName = LOCTEXT("JSONLiveLinkSourceMachineName", "localhost");

    // setup socket
    if (DeviceEndpoint.Address.IsMulticastAddress()) {
        Socket = FUdpSocketBuilder(TEXT("JSONSOCKET"))
            .AsNonBlocking()
            .AsReusable()
            .BoundToPort(DeviceEndpoint.Port)
            .WithReceiveBufferSize(RECV_BUFFER_SIZE)

            .BoundToAddress(FIPv4Address::Any)
            .JoinedToGroup(DeviceEndpoint.Address)
            .WithMulticastLoopback()
            .WithMulticastTtl(2);

    }
    else {
        Socket = FUdpSocketBuilder(TEXT("JSONSOCKET"))
            .AsNonBlocking()
            .AsReusable()
            .BoundToAddress(DeviceEndpoint.Address)
            .BoundToPort(DeviceEndpoint.Port)
            .WithReceiveBufferSize(RECV_BUFFER_SIZE);
    }

    RecvBuffer.SetNumUninitialized(RECV_BUFFER_SIZE);

    if ((Socket != nullptr) && (Socket->GetSocketType() == SOCKTYPE_Datagram)) {
        SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

        Start();

        SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
    }
}

FMoversePluginSource::~FMoversePluginSource() {
    Stop();
    if (Thread != nullptr) {
        Thread->WaitForCompletion();
        delete Thread;
        Thread = nullptr;
    }
    if (Socket != nullptr) {
        Socket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
    }
    while (receiving.load()) {
    };
}

void FMoversePluginSource::ReceiveClient(ILiveLinkClient* InClient,
    FGuid InSourceGuid) {
    Client = InClient;
    SourceGuid = InSourceGuid;
}

bool FMoversePluginSource::IsSourceStillValid() const {
    // Source is valid if we have a valid thread and socket
    bool bIsSourceValid = !Stopping && Thread != nullptr && Socket != nullptr;
    return bIsSourceValid;
}

bool FMoversePluginSource::RequestSourceShutdown() {
    Stop();

    return true;
}
// FRunnable interface

void FMoversePluginSource::Start() {
    ThreadName = "JSON UDP Receiver ";
    ThreadName.AppendInt(FAsyncThreadIndex::GetNext());

    Thread =
        FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal,
            FPlatformAffinity::GetPoolThreadMask());
}

void FMoversePluginSource::Stop() { Stopping = true; }

uint32 FMoversePluginSource::Run() {
    TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();

    while (!Stopping) {
        if (Socket->Wait(ESocketWaitConditions::WaitForRead, WaitTime)) {
            uint32 Size;

            while (Socket->HasPendingData(Size)) {
                int32 Read = 0;

                if (Socket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read,
                    *Sender)) {
                    if (Read > 0) {
                        TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData =
                            MakeShareable(new TArray<uint8>());
                        ReceivedData->SetNumUninitialized(Read);
                        memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
                        AsyncTask(ENamedThreads::GameThread, [this, ReceivedData]() {
                            HandleReceivedData(ReceivedData);
                            });
                    }
                }
            }
        }
    }
    return 0;
}

void FMoversePluginSource::HandleReceivedData(
    TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData) {
    // UE_LOG(ModuleLog, Warning, TEXT("HandleReceiveData"));
    receiving.store(true);
    FString JsonString;
    JsonString.Empty(ReceivedData->Num());
    for (uint8& Byte : *ReceivedData.Get()) {
        JsonString += TCHAR(Byte);
    }

    // type values from the JSON
    FString validtypes = FString(
        "CharacterSubject CharacterAnimation CameraSubject CameraAnimation "
        "LightSubject LightAnimation TransformSubject TransformAnimation");

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        for (TPair<FString, TSharedPtr<FJsonValue>> JsonField :
            JsonObject->Values) {
            FName SubjectName(*JsonField.Key);
            const TArray<TSharedPtr<FJsonValue>>& BoneArray =
                JsonField.Value->AsArray();

            // type of JSON data found in first value of the BoneArray
            // contents of BoneArray depends on the type - Character, Camera, Light,
            // Transform
            const TSharedPtr<FJsonValue>& MyType = BoneArray[0];
            const TSharedPtr<FJsonObject> MyTypeObject = MyType->AsObject();

            FString MyTypeName;
            if (MyTypeObject->TryGetStringField(TEXT("Type"), MyTypeName)) {
                if (validtypes.Contains(MyTypeName)) {
                    // UE_LOG(ModuleLog, Warning, TEXT("HandleReceiveData - object type:
                    // %s"), *FString(MyTypeName));
                }
                else {
                    UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid type: %s"),
                        *FString(MyTypeName));
                    receiving.store(false);
                    return;
                }

            }
            else {
                // Invalid Json Format
                UE_LOG(ModuleLog, Error,
                    TEXT("HandleReceiveData - invalid json 'Type' value not found"));
                receiving.store(false);
                return;
            }

            bool bCreateSubject = !EncounteredSubjects.Contains(SubjectName);

            // Camera Subject

            if (bCreateSubject && MyTypeName == "CameraSubject") {
                UE_LOG(ModuleLog, Warning, TEXT("HandleReceiveData - CameraSubject"));
                FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(
                    FLiveLinkCameraStaticData::StaticStruct());
                FLiveLinkCameraStaticData& CameraData =
                    *StaticDataStruct.Cast<FLiveLinkCameraStaticData>();

                const TSharedPtr<FJsonValue>& Cam = BoneArray[1];
                const TSharedPtr<FJsonObject> CamObject = Cam->AsObject();

                bool FieldOfView, AspectRatio, FocalLength, ProjectionMode;
                if (CamObject->TryGetBoolField(TEXT("FieldOfView"), FieldOfView)) {
                    if (FieldOfView)
                        CameraData.bIsFieldOfViewSupported = true;
                    else
                        CameraData.bIsFieldOfViewSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data,  FieldOfView value "
                            "not found."));
                    CameraData.bIsFieldOfViewSupported = false;
                }

                if (CamObject->TryGetBoolField(TEXT("AspectRatio"), AspectRatio)) {
                    if (AspectRatio)
                        CameraData.bIsAspectRatioSupported = true;
                    else
                        CameraData.bIsAspectRatioSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, AspectRatio value not "
                            "found."));
                    CameraData.bIsAspectRatioSupported = false;
                }

                if (CamObject->TryGetBoolField(TEXT("FocalLength"), FocalLength)) {
                    if (FocalLength)
                        CameraData.bIsFocalLengthSupported = true;
                    else
                        CameraData.bIsFocalLengthSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, FocalLength value not "
                            "found."));
                    CameraData.bIsFocalLengthSupported = false;
                }

                if (CamObject->TryGetBoolField(TEXT("ProjectionMode"),
                    ProjectionMode)) {
                    if (ProjectionMode)
                        CameraData.bIsProjectionModeSupported = true;
                    else
                        CameraData.bIsProjectionModeSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, ProjectionMode value "
                            "not found."));
                    CameraData.bIsProjectionModeSupported = false;
                }

                Client->PushSubjectStaticData_AnyThread(
                    { SourceGuid, SubjectName }, ULiveLinkCameraRole::StaticClass(),
                    MoveTemp(StaticDataStruct));
                EncounteredSubjects.Add(SubjectName);
            }

            // Light Subject

            if (bCreateSubject && MyTypeName == "LightSubject") {
                UE_LOG(ModuleLog, Warning, TEXT("HandleReceiveData - LightSubject"));
                FLiveLinkStaticDataStruct StaticDataStruct =
                    FLiveLinkStaticDataStruct(FLiveLinkLightStaticData::StaticStruct());
                FLiveLinkLightStaticData& LightData =
                    *StaticDataStruct.Cast<FLiveLinkLightStaticData>();

                const TSharedPtr<FJsonValue>& Lit = BoneArray[1];
                const TSharedPtr<FJsonObject> LitObject = Lit->AsObject();

                bool Intensity, LightColor, InnerConeAngle, OuterConeAngle;

                if (LitObject->TryGetBoolField(TEXT("Intensity"), Intensity)) {
                    if (Intensity)
                        LightData.bIsIntensitySupported = true;
                    else
                        LightData.bIsIntensitySupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, Intensity value not "
                            "found."));
                    LightData.bIsIntensitySupported = false;
                }

                if (LitObject->TryGetBoolField(TEXT("LightColor"), LightColor)) {
                    if (LightColor)
                        LightData.bIsLightColorSupported = true;
                    else
                        LightData.bIsLightColorSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, LightColor value not "
                            "found."));
                    LightData.bIsLightColorSupported = false;
                }

                if (LitObject->TryGetBoolField(TEXT("InnerConeAngle"),
                    InnerConeAngle)) {
                    if (InnerConeAngle)
                        LightData.bIsInnerConeAngleSupported = true;
                    else
                        LightData.bIsInnerConeAngleSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, InnerConeAngle value "
                            "not found."));
                    LightData.bIsInnerConeAngleSupported = false;
                }

                if (LitObject->TryGetBoolField(TEXT("InnerConeAngle"),
                    OuterConeAngle)) {
                    if (OuterConeAngle)
                        LightData.bIsOuterConeAngleSupported = true;
                    else
                        LightData.bIsOuterConeAngleSupported = false;
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid data, OuterConeAngle value "
                            "not found"));
                    LightData.bIsOuterConeAngleSupported = false;
                }

                Client->PushSubjectStaticData_AnyThread(
                    { SourceGuid, SubjectName }, ULiveLinkLightRole::StaticClass(),
                    MoveTemp(StaticDataStruct));
                EncounteredSubjects.Add(SubjectName);
            }

            // Transform Subject

            if (bCreateSubject && MyTypeName == "TransformSubject") {
                UE_LOG(ModuleLog, Warning,
                    TEXT("HandleReceiveData - TransformSubject"));
                FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(
                    FLiveLinkTransformStaticData::StaticStruct());

                Client->PushSubjectStaticData_AnyThread(
                    { SourceGuid, SubjectName }, ULiveLinkTransformRole::StaticClass(),
                    MoveTemp(StaticDataStruct));
                EncounteredSubjects.Add(SubjectName);
            }

            // Character Subject

            if (bCreateSubject && MyTypeName == "CharacterSubject") {
                UE_LOG(ModuleLog, Warning,
                    TEXT("HandleReceiveData - CharacterSubject"));
                FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(
                    FLiveLinkSkeletonStaticData::StaticStruct());
                FLiveLinkSkeletonStaticData* StaticDataPtr =
                    StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>();
                
                if (StaticDataPtr){
                    FLiveLinkSkeletonStaticData& StaticData = *StaticDataPtr;
                    StaticData.BoneNames.SetNumUninitialized(BoneArray.Num() - 1);
                    StaticData.BoneParents.SetNumUninitialized(BoneArray.Num() - 1);

                    for (int BoneIdx = 0; BoneIdx < BoneArray.Num() - 1; ++BoneIdx) {
                        const TSharedPtr<FJsonValue>& Bone = BoneArray[BoneIdx + 1];
                        const TSharedPtr<FJsonObject> BoneObject = Bone->AsObject();

                        FString BoneName;
                        if (BoneObject->TryGetStringField(TEXT("Name"), BoneName)) {
                            StaticData.BoneNames[BoneIdx] = FName(*BoneName);
                        }
                        else {
                            // Invalid Json Format
                            UE_LOG(ModuleLog, Error,
                                TEXT("HandleReceiveData - invalid bone name"));
                            receiving.store(false);
                            return;
                        }

                        int32 BoneParentIdx;
                        if (BoneObject->TryGetNumberField(TEXT("Parent"), BoneParentIdx)) {
                            StaticData.BoneParents[BoneIdx] = BoneParentIdx;
                        }
                        else {
                            // Invalid Json Format
                            UE_LOG(ModuleLog, Error,
                                TEXT("HandleReceiveData - invalid bone parent index"));
                            receiving.store(false);
                            return;
                        }
                    }

                    Client->PushSubjectStaticData_AnyThread(
                        { SourceGuid, SubjectName }, ULiveLinkAnimationRole::StaticClass(),
                        MoveTemp(StaticDataStruct));
                    EncounteredSubjects.Add(SubjectName);
                }
            }

            // Transform Animation

            if (MyTypeName == "TransformAnimation") {
                FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(
                    FLiveLinkTransformFrameData::StaticStruct());
                FLiveLinkTransformFrameData& FrameData =
                    *FrameDataStruct.Cast<FLiveLinkTransformFrameData>();

                const TSharedPtr<FJsonValue>& Transform = BoneArray[1];
                const TSharedPtr<FJsonObject> TransformObject = Transform->AsObject();

                const TArray<TSharedPtr<FJsonValue>>* LocationArray;
                FVector TransformLocation;
                if (TransformObject->TryGetArrayField(TEXT("Location"),
                    LocationArray) &&
                    LocationArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*LocationArray)[0]->AsNumber();
                    double Y = (*LocationArray)[1]->AsNumber();
                    double Z = (*LocationArray)[2]->AsNumber();
                    TransformLocation = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid transform location"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* RotationArray;
                FQuat TransformQuat;
                if (TransformObject->TryGetArrayField(TEXT("Rotation"),
                    RotationArray) &&
                    RotationArray->Num() == 4)  // X, Y, Z, W
                {
                    double X = (*RotationArray)[0]->AsNumber();
                    double Y = (*RotationArray)[1]->AsNumber();
                    double Z = (*RotationArray)[2]->AsNumber();
                    double W = (*RotationArray)[3]->AsNumber();
                    TransformQuat = FQuat(X, Y, Z, W);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid transform rotation"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* ScaleArray;
                FVector TransformScale;
                if (TransformObject->TryGetArrayField(TEXT("Scale"), ScaleArray) &&
                    ScaleArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*ScaleArray)[0]->AsNumber();
                    double Y = (*ScaleArray)[1]->AsNumber();
                    double Z = (*ScaleArray)[2]->AsNumber();
                    TransformScale = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid transform scale"));
                    receiving.store(false);
                    return;
                }

                FrameData.Transform =
                    FTransform(TransformQuat, TransformLocation, TransformScale);

                Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName },
                    MoveTemp(FrameDataStruct));
            }

            // Camera Animation

            if (MyTypeName == "CameraAnimation") {
                FLiveLinkFrameDataStruct FrameDataStruct =
                    FLiveLinkFrameDataStruct(FLiveLinkCameraFrameData::StaticStruct());
                FLiveLinkCameraFrameData& FrameData =
                    *FrameDataStruct.Cast<FLiveLinkCameraFrameData>();

                const TSharedPtr<FJsonValue>& Camera = BoneArray[1];
                const TSharedPtr<FJsonObject> CameraObject = Camera->AsObject();

                const TArray<TSharedPtr<FJsonValue>>* LocationArray;
                FVector CameraLocation;
                if (CameraObject->TryGetArrayField(TEXT("Location"), LocationArray) &&
                    LocationArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*LocationArray)[0]->AsNumber();
                    double Y = (*LocationArray)[1]->AsNumber();
                    double Z = (*LocationArray)[2]->AsNumber();
                    CameraLocation = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid camera location"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* RotationArray;
                FQuat CameraQuat;
                if (CameraObject->TryGetArrayField(TEXT("Rotation"), RotationArray) &&
                    RotationArray->Num() == 4)  // X, Y, Z, W
                {
                    double X = (*RotationArray)[0]->AsNumber();
                    double Y = (*RotationArray)[1]->AsNumber();
                    double Z = (*RotationArray)[2]->AsNumber();
                    double W = (*RotationArray)[3]->AsNumber();
                    CameraQuat = FQuat(X, Y, Z, W);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid camera rotation"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* ScaleArray;
                FVector CameraScale;
                if (CameraObject->TryGetArrayField(TEXT("Scale"), ScaleArray) &&
                    ScaleArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*ScaleArray)[0]->AsNumber();
                    double Y = (*ScaleArray)[1]->AsNumber();
                    double Z = (*ScaleArray)[2]->AsNumber();
                    CameraScale = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    // UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid camera
                    // scale")); return;
                    CameraScale = FVector(1, 1, 1);
                }

                FrameData.Transform =
                    FTransform(CameraQuat, CameraLocation, CameraScale);

                const TArray<TSharedPtr<FJsonValue>>* CamValuesArray;
                if (CameraObject->TryGetArrayField(TEXT("Values"), CamValuesArray) &&
                    CamValuesArray->Num() == 3)  // X, Y, Z
                {
                    double horizontalFieldOfView = (*CamValuesArray)[0]->AsNumber();
                    double aspectRatio = (*CamValuesArray)[1]->AsNumber();
                    double focalLength = (*CamValuesArray)[2]->AsNumber();

                    FrameData.FieldOfView = horizontalFieldOfView;
                    FrameData.AspectRatio = aspectRatio;
                    FrameData.FocalLength = focalLength;
                    // FrameData.ProjectionMode =
                    // ELiveLinkCameraProjectionMode::Perspective;
                }
                // else
                //{
                //	// Invalid Json Format
                //	UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid
                // camera attributes")); 	return;
                // }

                Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName },
                    MoveTemp(FrameDataStruct));
            }

            // Camera Animation

            if (MyTypeName == "LightAnimation") {
                FLiveLinkFrameDataStruct FrameDataStruct =
                    FLiveLinkFrameDataStruct(FLiveLinkLightFrameData::StaticStruct());
                FLiveLinkLightFrameData& FrameData =
                    *FrameDataStruct.Cast<FLiveLinkLightFrameData>();

                const TSharedPtr<FJsonValue>& Light = BoneArray[1];
                const TSharedPtr<FJsonObject> LightObject = Light->AsObject();

                const TArray<TSharedPtr<FJsonValue>>* LocationArray;
                FVector LightLocation;
                if (LightObject->TryGetArrayField(TEXT("Location"), LocationArray) &&
                    LocationArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*LocationArray)[0]->AsNumber();
                    double Y = (*LocationArray)[1]->AsNumber();
                    double Z = (*LocationArray)[2]->AsNumber();
                    LightLocation = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid light location"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* RotationArray;
                FQuat LightQuat;
                if (LightObject->TryGetArrayField(TEXT("Rotation"), RotationArray) &&
                    RotationArray->Num() == 4)  // X, Y, Z, W
                {
                    double X = (*RotationArray)[0]->AsNumber();
                    double Y = (*RotationArray)[1]->AsNumber();
                    double Z = (*RotationArray)[2]->AsNumber();
                    double W = (*RotationArray)[3]->AsNumber();
                    LightQuat = FQuat(X, Y, Z, W);
                }
                else {
                    // Invalid Json Format
                    UE_LOG(ModuleLog, Error,
                        TEXT("HandleReceiveData - invalid light rotation"));
                    receiving.store(false);
                    return;
                }

                const TArray<TSharedPtr<FJsonValue>>* ScaleArray;
                FVector LightScale;
                if (LightObject->TryGetArrayField(TEXT("Scale"), ScaleArray) &&
                    ScaleArray->Num() == 3)  // X, Y, Z
                {
                    double X = (*ScaleArray)[0]->AsNumber();
                    double Y = (*ScaleArray)[1]->AsNumber();
                    double Z = (*ScaleArray)[2]->AsNumber();
                    LightScale = FVector(X, Y, Z);
                }
                else {
                    // Invalid Json Format
                    // UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid light
                    // scale")); return;
                    LightScale = FVector(1, 1, 1);
                }

                FrameData.Transform = FTransform(LightQuat, LightLocation, LightScale);

                const TArray<TSharedPtr<FJsonValue>>* IntensityArray;
                if (LightObject->TryGetArrayField(TEXT("Intensity"), IntensityArray) &&
                    IntensityArray->Num() == 1)  // Intensity
                {
                    double Intensity = (*IntensityArray)[0]->AsNumber();
                    FrameData.Intensity = Intensity;
                }
                // else
                //{
                //	// Invalid Json Format
                //	UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid light
                // intensity")); 	return;
                // }

                const TArray<TSharedPtr<FJsonValue>>* ColorArray;
                if (LightObject->TryGetArrayField(TEXT("LightColor"), ColorArray) &&
                    ColorArray->Num() == 3)  // Intensity
                {
                    uint8 R = (*ColorArray)[0]->AsNumber();
                    uint8 G = (*ColorArray)[1]->AsNumber();
                    uint8 B = (*ColorArray)[2]->AsNumber();
                    FrameData.LightColor = FColor(R, G, B, 255);
                }
                // else
                //{
                //	// Invalid Json Format
                //	UE_LOG(ModuleLog, Error, TEXT("HandleReceiveData - invalid light
                // color")); 	return;
                // }

                const TArray<TSharedPtr<FJsonValue>>* ConeArray;
                if (LightObject->TryGetArrayField(TEXT("Angle"), ConeArray) &&
                    ConeArray->Num() == 2)  // InnerConeAngle, OuterConeAngle
                {
                    double InnerConeAngle = (*ConeArray)[0]->AsNumber();
                    double OuterConeAngle = (*ConeArray)[1]->AsNumber();
                    FrameData.InnerConeAngle = InnerConeAngle;
                    FrameData.OuterConeAngle = OuterConeAngle;
                }
                // else
                //{
                //	// Invalid Json Format
                //	UE_LOG(ModuleLog, Warning, TEXT("HandleReceiveData - invalid
                // light angle or not a spotlight"));
                // }

                Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName },
                    MoveTemp(FrameDataStruct));
            }

            // Character Animation

            if (MyTypeName == "CharacterAnimation") {
                FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(
                    FLiveLinkAnimationFrameData::StaticStruct());

                auto FrameDataStructPtr = FrameDataStruct.Cast<FLiveLinkAnimationFrameData>();
                if (FrameDataStructPtr)
                {
                    FLiveLinkAnimationFrameData& FrameData = *FrameDataStructPtr;

                    FrameData.Transforms.SetNumUninitialized(BoneArray.Num() - 1);

                    for (int BoneIdx = 0; BoneIdx < BoneArray.Num() - 1; ++BoneIdx) {
                        const TSharedPtr<FJsonValue>& Bone = BoneArray[BoneIdx + 1];
                        const TSharedPtr<FJsonObject> BoneObject = Bone->AsObject();

                        const TArray<TSharedPtr<FJsonValue>>* LocationArray;
                        FVector BoneLocation;
                        if (BoneObject->TryGetArrayField(TEXT("Location"), LocationArray) &&
                            LocationArray->Num() == 3)  // X, Y, Z
                        {
                            double X = (*LocationArray)[0]->AsNumber();
                            double Y = (*LocationArray)[1]->AsNumber();
                            double Z = (*LocationArray)[2]->AsNumber();
                            BoneLocation = FVector(X, Y, Z);
                        }
                        else {
                            // Invalid Json Format
                            UE_LOG(ModuleLog, Error,
                                TEXT("HandleReceiveData - index[%i] invalid location"),
                                BoneIdx);
                            receiving.store(false);
                            return;
                        }

                        const TArray<TSharedPtr<FJsonValue>>* RotationArray;
                        FQuat BoneQuat;
                        if (BoneObject->TryGetArrayField(TEXT("Rotation"), RotationArray) &&
                            RotationArray->Num() == 4)  // X, Y, Z, W
                        {
                            double X = (*RotationArray)[0]->AsNumber();
                            double Y = (*RotationArray)[1]->AsNumber();
                            double Z = (*RotationArray)[2]->AsNumber();
                            double W = (*RotationArray)[3]->AsNumber();
                            BoneQuat = FQuat(X, Y, Z, W);

                            if (BoneIdx == 0) {
                                FQuat rot90 = FQuat(0.7071068, 0, 0, 0.7071068);
                                BoneQuat = rot90 * BoneQuat;
                            }
                        }
                        else {
                            // Invalid Json Format
                            UE_LOG(ModuleLog, Error,
                                TEXT("HandleReceiveData - index[%i] invalid rotation"),
                                BoneIdx);
                            receiving.store(false);
                            return;
                        }

                        const TArray<TSharedPtr<FJsonValue>>* ScaleArray;
                        FVector BoneScale;
                        if (BoneObject->TryGetArrayField(TEXT("Scale"), ScaleArray) &&
                            ScaleArray->Num() == 3)  // X, Y, Z
                        {
                            double X = (*ScaleArray)[0]->AsNumber();
                            double Y = (*ScaleArray)[1]->AsNumber();
                            double Z = (*ScaleArray)[2]->AsNumber();
                            BoneScale = FVector(X, Y, Z);
                        }
                        else {
                            // Invalid Json Format
                            UE_LOG(ModuleLog, Error,
                                TEXT("HandleReceiveData - index[%i] invalid scale"),
                                BoneIdx);
                            receiving.store(false);
                            return;
                        }

                        FrameData.Transforms[BoneIdx] =
                            FTransform(BoneQuat, BoneLocation, BoneScale);
                    }

                    Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName },
                        MoveTemp(FrameDataStruct));

                }
            }
        }
    }
    receiving.store(false);
}

bool FMoversePluginSource::ImportFBXAsset(const FString& FilePath,
    const FString& DestinationPath) {
#if WITH_EDITOR
    if (GEditor) {
        // Get the Asset Tools module
        FAssetToolsModule& AssetToolsModule =
            FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        IAssetTools& AssetTools = AssetToolsModule.Get();

        // Set import options
        UFbxFactory* FbxFactory =
            NewObject<UFbxFactory>(UFbxFactory::StaticClass());
        FbxFactory->AddToRoot();  // Prevent garbage collection

        if (FbxFactory->ImportUI) {
            // Set import options to avoid showing the import dialog

            // Disable animation import
            FbxFactory->ImportUI->bImportAnimations = false;
            FbxFactory->ImportUI->bAutomatedImportShouldDetectType = true;
            FbxFactory->ImportUI->bCreatePhysicsAsset = false;

            // Enable skeletal mesh import
            FbxFactory->ImportUI->bImportAsSkeletal = true;
        }

        // Specify where the asset should be saved
        FString PackagePath = TEXT("/Game/") + DestinationPath;

        // Call the import asset function
        TArray<UObject*> ImportedAssets =
            AssetTools.ImportAssets({ FilePath }, PackagePath, FbxFactory, true);

        // Log the results
        if (ImportedAssets.Num() <= 0) {
            return false;
        }

        UE_LOG(LogTemp, Log, TEXT("Successfully imported FBX asset from %s"),
            *FilePath);

        USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ImportedAssets[0]);

        UAnimBlueprint* AnimBlueprint =
            CreateAnimBlueprint(SkeletalMesh, TEXT("asset_Animation"), PackagePath);

        AnimBlueprint->AddToRoot();

        bool auto_import_mesh = false;

        if (auto_import_mesh) {
            FWorldContext* world =
                GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);

            UWorld* World = world->World();

            if (!World) {
                GEngine->AddOnScreenDebugMessage(
                    -1, 15.0f, FColor::Yellow, TEXT("GEngine->GetWorld() return NULL"));
                return false;
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = nullptr;

            FVector SpawnLocation =
                FVector(0.0f, 0.0f, 100.0f);  // Customize spawn location
            FRotator SpawnRotation =
                FRotator(0.0f, 0.0f, 0.0f);  // Customize spawn rotation

            // Spawn SkeletalMeshActor in the world
            ASkeletalMeshActor* SkeletalMeshActor =
                World->SpawnActor<ASkeletalMeshActor>(SpawnLocation, SpawnRotation,
                    SpawnParams);
            SkeletalMeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(
                SkeletalMesh);
            SkeletalMeshActor->GetSkeletalMeshComponent()->SetAnimInstanceClass(
                AnimBlueprint->GetClass());

            if (AnimBlueprint == nullptr) {
                UE_LOG(LogTemp, Error, TEXT("Failed to create Animation Blueprint"));
                return false;
            }
        }

        /*} else {
          UE_LOG(LogTemp, Error, TEXT("Failed to import FBX asset from %s"),
                 *FilePath);
          return false;*/
        FbxFactory->RemoveFromRoot();  // Allow garbage collection after import
        return true;
    }

#endif
    return false;
}

UAnimBlueprint* FMoversePluginSource::CreateAnimBlueprint(
    USkeletalMesh* SkeletalMesh, const FString& BlueprintName,
    const FString& Path) {
#if WITH_EDITOR
    if (!SkeletalMesh || !SkeletalMesh->Skeleton) {
        UE_LOG(LogTemp, Error, TEXT("Invalid SkeletalMesh or Skeleton."));
        return nullptr;
    }

    // Get the Skeleton from the Skeletal Mesh
    USkeleton* Skeleton = SkeletalMesh->Skeleton;

    // Ensure the path is valid and begins with a slash ("/")
    FString PackageName = FString::Printf(TEXT("%s/%s"), *Path, *BlueprintName);
    FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

    UPackage* Package = CreatePackage(*PackageName);
    if (!Package) {
        UE_LOG(LogTemp, Error,
            TEXT("Failed to create package for Animation Blueprint."));
        return nullptr;
    }

    // Define the parent class for the Animation Blueprint (typically
    // UAnimInstance)
    UClass* ParentClass = UAnimInstance::StaticClass();

    // Create the Animation Blueprint with the factory method
    UAnimBlueprint* NewAnimBlueprint =
        Cast<UAnimBlueprint>(FKismetEditorUtilities::CreateBlueprint(
            ParentClass,            // The parent class (UAnimInstance)
            Package,                // The package where the asset will be stored
            FName(*BlueprintName),  // The blueprint's name
            BPTYPE_Normal,          // The blueprint type
            UAnimBlueprint::StaticClass(),  // The type of blueprint to create
            // (Anim Blueprint)
            UBlueprintGeneratedClass::StaticClass(),  // The class generated by
            // the blueprint
            FName("CreateAnimBlueprint")  // Log name for the creation process
        ));

    if (!NewAnimBlueprint) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Animation Blueprint."));
        return nullptr;
    }

    // Set the target skeleton for the new animation blueprint
    NewAnimBlueprint->TargetSkeleton = Skeleton;

    // Compile the animation blueprint to finalize it
    FKismetEditorUtilities::CompileBlueprint(NewAnimBlueprint);

    // Mark the package dirty so it can be saved later
    Package->MarkPackageDirty();

    // Save the package to disk
    FAssetRegistryModule::AssetCreated(NewAnimBlueprint);
    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        PackageName, FPackageName::GetAssetPackageExtension());
    /*if (!UPackage::SavePackage(
            Package, NewAnimBlueprint,
            EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
            *PackageFileName)) {
      UE_LOG(LogTemp, Error, TEXT("Failed to save Animation Blueprint package."));
    }*/

    return NewAnimBlueprint;
#endif
    return nullptr;
}

bool FMoversePluginSource::LoadAndConnectAssets() {
    if (this->ActorName.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("Invalid actor name %s"), *(ActorName));
        return false;
    }

    FString PluginPath =
        IPluginManager::Get().FindPlugin("MoversePlugin")->GetBaseDir();
    UE_LOG(LogTemp, Log, TEXT("BP Path %s"), *(PluginPath / "Content"));

    if (!ImportFBXAsset(PluginPath / "Content" / "asset.fbx", ActorName)) {
        return false;
    }

    return true;
}

#undef LOCTEXT_NAMESPACE
