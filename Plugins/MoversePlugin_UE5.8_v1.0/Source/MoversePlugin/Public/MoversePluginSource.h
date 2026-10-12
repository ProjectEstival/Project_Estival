// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <atomic>

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "ILiveLinkSource.h"
#include "IMessageContext.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

class FRunnableThread;
class FSocket;
class ILiveLinkClient;
class ISocketSubsystem;

DECLARE_LOG_CATEGORY_EXTERN(ModuleLog, Log, All)
// JSONLIVELINK_API
class FMoversePluginSource : public ILiveLinkSource, public FRunnable {
 public:
  FMoversePluginSource(FIPv4Endpoint Endpoint, FString ActorName,
                       bool ImportAssets);

  virtual ~FMoversePluginSource();

  // Begin ILiveLinkSource Interface

  virtual void ReceiveClient(ILiveLinkClient* InClient,
                             FGuid InSourceGuid) override;

  virtual bool IsSourceStillValid() const override;

  virtual bool RequestSourceShutdown() override;

  virtual FText GetSourceType() const override { return SourceType; };
  virtual FText GetSourceMachineName() const override {
    return SourceMachineName;
  }
  virtual FText GetSourceStatus() const override { return SourceStatus; }

  // End ILiveLinkSource Interface

  // Begin FRunnable Interface

  virtual bool Init() override { return true; }
  virtual uint32 Run() override;
  void Start();
  virtual void Stop() override;
  virtual void Exit() override {}

  // End FRunnable Interface

  void HandleReceivedData(
      TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

 private:
  ILiveLinkClient* Client;

  // Our identifier in LiveLink
  FGuid SourceGuid;

  FMessageAddress ConnectionAddress;

  FText SourceType;
  FText SourceMachineName;
  FText SourceStatus;

  FIPv4Endpoint DeviceEndpoint;

  // Socket to receive data on
  FSocket* Socket;

  // Subsystem associated to Socket
  ISocketSubsystem* SocketSubsystem;

  // Threadsafe Bool for terminating the main thread loop
  FThreadSafeBool Stopping;

  // Thread to run socket operations on
  FRunnableThread* Thread;

  // Name of the sockets thread
  FString ThreadName;

  // Time to wait between attempted receives
  FTimespan WaitTime;

  // List of subjects we've already encountered
  TSet<FName> EncounteredSubjects;

  // Buffer to receive socket data into
  TArray<uint8> RecvBuffer;

  // Name of the actor
  FString ActorName;

  bool ImportAssets;
  std::atomic_bool receiving{false};

 private:
  bool LoadAndConnectAssets();
  bool ImportFBXAsset(const FString& FilePath, const FString& DestinationPath);
  UAnimBlueprint* CreateAnimBlueprint(USkeletalMesh* SkeletalMesh,
                                      const FString& BlueprintName,
                                      const FString& Path);
};
