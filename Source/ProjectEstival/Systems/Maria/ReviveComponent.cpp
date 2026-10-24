// Fill out your copyright notice in the Description page of Project Settings.


#include "ReviveComponent.h"
#include "Revivable.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"


UReviveComponent::UReviveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UReviveComponent::BeginPlay()
{
	Super::BeginPlay();
	RemainingLives = FMath::Max(0, MaxLives);
	OnLivesChanged.Broadcast(RemainingLives);
	
	if (AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner()))
	{
		Character->OnDeath.AddDynamic(this, &UReviveComponent::HandleOwnerDeath);
		Character->OnRevived.AddDynamic(this, &UReviveComponent::HandleOwnerRevived);
	}
}

void UReviveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelResurrection();
	if (UWorld* World = GetWorld())World->GetTimerManager().ClearTimer(OffscreenTimer);
	
	Super::EndPlay(EndPlayReason);
}

bool UReviveComponent::CanBeRevived() const
{
	const AEstivalBaseCharacter* Owner = Cast<AEstivalBaseCharacter>(GetOwner());
	return Owner && !Owner-> IsAlive() && RemainingLives > 0 && !bIsInProgress;
}

bool UReviveComponent::BeginResurrection(AActor* Reviver)
{
	if (!CanBeRevived() || !GetWorld()) return false;
	
	CurrentReviver = Reviver;
	bIsInProgress = true;
	OnStarted.Broadcast();
	
	if (Duration <= 0.f)
	{
		Complete();
		return true;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UReviveComponent::Complete,
		Duration,
		false);
	return true;
}

void UReviveComponent::CancelResurrection()
{
	if (!bIsInProgress) return;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	
	bIsInProgress = false;
	CurrentReviver.Reset();
	OnCancelled.Broadcast();
}

float UReviveComponent::GetProgress() const
{
	if (!bIsInProgress || Duration <= 0.f || !GetWorld()) return 0.f;
	
	const float Remaining = FMath::Max(0.f, GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle));
	return FMath::Clamp(1.f - Remaining / Duration, 0.f, 1.f);
}

void UReviveComponent::Complete()
{
	bIsInProgress = false;
	TimerHandle.Invalidate();
	CurrentReviver.Reset();
	
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<UReviveComponent>() && IRevivable::Execute_TryRevive(Owner, HealthPercent))
	{
		RemainingLives--;
		OnLivesChanged.Broadcast(RemainingLives);
		OnCompleted.Broadcast();
		return;
	}
	
	OnCancelled.Broadcast();
}

void UReviveComponent::HandleOwnerDeath()
{
	OffscreenTime = 0.f;
	GetWorld()->GetTimerManager().SetTimer(
		OffscreenTimer,
		this,
		&UReviveComponent::CheckOffScreen,
		OffscreenCheckInterval,
		true);
}

void UReviveComponent::HandleOwnerRevived()
{
	GetWorld()->GetTimerManager().ClearTimer(OffscreenTimer);
}

void UReviveComponent::CheckOffScreen()
{
	if (!bIsInProgress || RemainingLives <= 0 || IsOwnerOnScreen())
	{
		OffscreenTime = 0.f;
		return;
	}
	
	OffscreenTime += OffscreenCheckInterval;
	if (OffscreenTime < OffScreenDelay) return;
	
	OffscreenTime = 0.f;
	FVector Location;
	if (FindPartyLocation(Location)) ReviveAtLocation(Location);
}

void UReviveComponent::ReviveAtLocation(const FVector& Location)
{
	if (!CanBeRevived()) return;
	
	GetOwner()->SetActorLocation(Location,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	Complete();
}

bool UReviveComponent::IsOwnerOnScreen() const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
	if (!PC) return true;
	
	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	
	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(GetOwner()->GetActorLocation(),ScreenPos)) return false;
	
	return ScreenPos.X >= 0.f && ScreenPos.X <= SizeX && ScreenPos.Y >= 0.f && ScreenPos.Y <= SizeY;
}

bool UReviveComponent::FindPartyLocation(FVector& OutLocation) const
{
	float BestDistSq = TNumericLimits<float>::Max();
	bool bFound = false;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const AEstivalBaseCharacter* Teammate = PC ? Cast<AEstivalBaseCharacter>(PC->GetPawn()) : nullptr;
		if (!Teammate || !Teammate->IsAlive() || Teammate == GetOwner()) continue;
		
		const float DistSq = FVector::DistSquared(Teammate->GetActorLocation(), GetOwner()->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			OutLocation = Teammate->GetActorLocation();
			bFound = true;
		}
	}
	return bFound;
}