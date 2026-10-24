// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "Engine/OverlapResult.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"

APlayerBase::APlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	ResurrectionComponent = CreateDefaultSubobject<UReviveComponent>(TEXT("Resurrection"));
}

void APlayerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopRevive();
	Super::EndPlay(EndPlayReason);
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input || !ReviveAction) return;
	
	Input->BindAction(ReviveAction, ETriggerEvent::Started, this, &APlayerBase::StartRevive);
	Input->BindAction(ReviveAction, ETriggerEvent::Completed, this, &APlayerBase::StopRevive);
	Input->BindAction(ReviveAction,ETriggerEvent::Canceled,this, &APlayerBase::StopRevive);
}

void APlayerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const UReviveComponent* Target = ReviveTarget.Get();
	const bool bValid = IsAlive() && Target && Target->IsInProgress()
	&& FVector::DistSquared(GetActorLocation(), Target->GetOwner()->GetActorLocation()) <= FMath::Square(ReviveRange);
	
	if (!bValid) StopRevive();
}

void APlayerBase::StartRevive()
{
	if (!IsAlive() || ReviveTarget.IsValid()) return;
	
	UReviveComponent* Target = FindReviveTarget();
	if (!Target || !Target->BeginResurrection(this)) return;
	
	ReviveTarget = Target;
	SetActorTickEnabled(true);
}

void APlayerBase::StopRevive()
{
	if (UReviveComponent* Target = ReviveTarget.Get()) Target->CancelResurrection();
	
	ReviveTarget.Reset();
	SetActorTickEnabled(false);
}

UReviveComponent* APlayerBase::FindReviveTarget() const
{
	if (!GetWorld()) return nullptr;
	
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(
		Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ReviveRange),
		FCollisionQueryParams(SCENE_QUERY_STAT(FindReviveTarget), false, this));
	
	UReviveComponent* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		UReviveComponent* Candidate = Actor ? Actor->FindComponentByClass<UReviveComponent>() : nullptr;
		if (!Candidate || !Candidate->CanBeRevived()) continue;
		
		const float DistSq = FVector::DistSquared(GetActorLocation(), Actor->GetActorLocation());
		
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}
	return Best;
}