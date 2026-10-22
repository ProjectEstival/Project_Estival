// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

APlayerBase::APlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
	RemainingRevives = FMath::Max(0, MaxRevives);
	OnReviveCountChanged.Broadcast(RemainingRevives);
}

void APlayerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelResurrection();
	Super::EndPlay(EndPlayReason);
}

bool APlayerBase::BeginResurrection()
{
	return BeginResurrectionWithSettings(
		DefaultResurrectionDuration,
		DefaultResurrectionHealthPercent);
}

bool APlayerBase::BeginResurrectionWithSettings(const float DurationSeconds, const float HealthPercent)
{
	if (bIsResurrectionInProgress || !CanBeRevived_Implementation() || !GetWorld()) return false;
	
	PendingHealthPercent = FMath::Clamp(HealthPercent,0.0f, 1.0f);
	bIsResurrectionInProgress = true;
	OnResurrectionStarted.Broadcast();
	
	if (DurationSeconds <= 0.0f)
	{
		CompleteResurrection();
		return true;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		ResurrectionTimerHandle,
		this,
		&APlayerBase::CompleteResurrection,
		DurationSeconds,
		false);
	
	return true;
}

void APlayerBase::CancelResurrection()
{
	if (!bIsResurrectionInProgress) return;
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResurrectionTimerHandle);
	}
	
	bIsResurrectionInProgress = false;
	OnResurrectionCancelled.Broadcast();
}

float APlayerBase::GetResurrectionTimeRemaining() const
{
	if (!bIsResurrectionInProgress || !GetWorld()) return 0.0f;
	
	return GetWorld()->GetTimerManager().GetTimerRemaining(ResurrectionTimerHandle);
}

bool APlayerBase::CanBeRevived_Implementation() const
{
	return !IsAlive() && RemainingRevives > 0;
}

bool APlayerBase::ReviveCharacter_Implementation(const float HealthPercent)
{
	if (!CanBeRevived_Implementation()) return false;
	
	// The base class owns the actual character recovery and controller resurrection.
	// Only spend a revival if its revive operation reports success.
	if (!TryRevive(FMath::Clamp(HealthPercent,0.0f, 1.0f))) return false;
	
	RemainingRevives = FMath::Max(0, RemainingRevives - 1);
	OnReviveCountChanged.Broadcast(RemainingRevives);
	return true;
}
 void APlayerBase::CompleteResurrection()
{
	if (!bIsResurrectionInProgress) return;
	
	bIsResurrectionInProgress = false;
	ResurrectionTimerHandle.Invalidate();
	
	if (ReviveCharacter_Implementation(PendingHealthPercent)) OnResurrectionCompleted.Broadcast();
	else OnResurrectionCancelled.Broadcast(); // ← A failed attempt never consumes a revival
}