// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Core/Camera/EstivalSharedCamera.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// adding this to be able to filter the logs
DEFINE_LOG_CATEGORY_STATIC(LogEstivalCamera, Log, All);

// Sets default values
AEstivalSharedCamera::AEstivalSharedCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// fix to stop the camera lagging a frame behind 
 	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FocusPoint"));
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);	
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = MinDistance;
	
	// walls must not push the camera in front of players
	CameraBoom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

}

// Called when the game starts or when spawned
void AEstivalSharedCamera::BeginPlay()
{
	Super::BeginPlay();
	
	// disabling split screen
	UGameplayStatics::SetForceDisableSplitscreen(this, true);
	
	for (FConstPlayerControllerIterator It = GetWorld()-> GetPlayerControllerIterator(); It; ++It)
	{
		TakeOverview(It->Get());
	}
	
	PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &AEstivalSharedCamera::HandlePostLogin);
	
	// starting camera already framed.
	UpdateCamera(0.f, true);
}

// Called every frame
void AEstivalSharedCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCamera(DeltaTime, false);

}

void AEstivalSharedCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
	Super::EndPlay(EndPlayReason);
}

void AEstivalSharedCamera::HandlePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	// the event is global, so ignore other worlds (eg a second PIE window)
	if (GameMode && GameMode->GetWorld() == GetWorld())
	{
		TakeOverview(NewPlayer);
	}
}

void AEstivalSharedCamera::TakeOverview(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	// by default every Possess (spawn, revive) moves the view back to the pawn
	PlayerController->bAutoManageActiveCameraTarget = false;
	PlayerController->SetViewTarget(this);
}


void AEstivalSharedCamera::SetFocusOnTarget(AActor* NewFocusTarget)
{
	// adding no snap -> the camera glides to the new target with FollowSpeed
	FocusTarget = NewFocusTarget;
}

void AEstivalSharedCamera::GatherFramedActors(TArray<AActor*>& OutActors) const
{
	OutActors.Reset();
	
	// filtering through controllers to make sure only human characters are framed
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (PlayerController && PlayerController->GetPawn())
		{
			OutActors.Add(PlayerController->GetPawn());
		}
	}
}

FVector AEstivalSharedCamera::ComputeFocusLocation(const TArray<AActor*>& Actors) const
{
	if (IsValid(FocusTarget))
	{
		return FocusTarget->GetActorLocation();
	}

	// nobody to frame, stay where we are
	if (Actors.IsEmpty())
	{
		return GetActorLocation();
	}

	FVector Sum = FVector::ZeroVector;
	for (const AActor* Actor : Actors)
	{
		Sum += Actor->GetActorLocation();
	}
	return Sum / Actors.Num();
}

float AEstivalSharedCamera::DistanceToFit(const FVector& FocusLocation, const FVector& WorldLocation) const
{
	// the location in camera space: X->depth, Y->screen right, Z->screen up
	const FVector Local = CameraBoom->GetComponentRotation().UnrotateVector(WorldLocation - FocusLocation);

	// Unreal's FieldOfView is horizontal so the vertical one depends on the screen shape
	const float TanHalfHorizontalFOV = FMath::Tan(FMath::DegreesToRadians(Camera->FieldOfView * 0.5f));
	const float TanHalfVerticalFOV = TanHalfHorizontalFOV / GetViewportAspectRatio();

	const float DistanceForWidth = (FMath::Abs(Local.Y) + ScreenEdgePadding) / TanHalfHorizontalFOV - Local.X;
	const float DistanceForHeight = (FMath::Abs(Local.Z) + ScreenEdgePadding) / TanHalfVerticalFOV - Local.X;

	return FMath::Max(DistanceForWidth, DistanceForHeight);
}

float AEstivalSharedCamera::ComputeRequiredDistance(const FVector& FocusLocation, const TArray<AActor*>& Actors) const
{
	float Required = MinDistance;
	for (const AActor* Actor : Actors)
	{
		Required = FMath::Max(Required, DistanceToFit(FocusLocation, Actor->GetActorLocation()));
	}

	// Max wins over Min if a designer sets them the wrong way round
	return FMath::Min(Required, FMath::Max(MinDistance, MaxDistance));
}


float AEstivalSharedCamera::GetViewportAspectRatio() const
{
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (const UGameViewportClient* GameViewport = GetWorld()->GetGameViewport())
	{
		GameViewport->GetViewportSize(ViewportSize);
	}

	// in case no viewport yet (first frame): assume 16:9
	return ViewportSize.Y > 0.f ? ViewportSize.X / ViewportSize.Y : 16.f / 9.f;
}

bool AEstivalSharedCamera::CanFrameLocation(const FVector& WorldLocation) const
{
	return DistanceToFit(GetActorLocation(), WorldLocation) <= MaxDistance;
}

void AEstivalSharedCamera::UpdateCamera(float DeltaSeconds, bool bSnap)
{
	TArray<AActor*> FramedActors;
	GatherFramedActors(FramedActors);

	//move the focus point
	const FVector TargetFocus = ComputeFocusLocation(FramedActors);
	const FVector NewFocus = bSnap ? TargetFocus : FMath::VInterpTo(GetActorLocation(), TargetFocus, DeltaSeconds, FollowSpeed);
	SetActorLocation(NewFocus);

	// pull back or move in until everyone fits around the new focus
	const float TargetDistance = ComputeRequiredDistance(NewFocus, FramedActors);
	const float CurrentDistance = CameraBoom->TargetArmLength;
	const float ZoomSpeed = TargetDistance > CurrentDistance ? ZoomOutSpeed : ZoomInSpeed;
	CameraBoom->TargetArmLength = bSnap ? TargetDistance : FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaSeconds, ZoomSpeed);

	
}