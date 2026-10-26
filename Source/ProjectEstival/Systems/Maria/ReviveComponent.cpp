#include "ReviveComponent.h"
#include "Revivable.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

UReviveComponent::UReviveComponent()
{
	// only ticks while holding a revive
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UReviveComponent::BeginPlay()
{
	Super::BeginPlay();
	RemainingLives = FMath::Max(0, MaxLives);
	OnLivesChanged.Broadcast(RemainingLives);
}

void UReviveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelResurrection();
	StopReviveNearby();
	Super::EndPlay(EndPlayReason);
}

void UReviveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner());
	const UReviveComponent* Target = ReviveTarget.Get();

	const bool bValid = Character && Character->IsAlive() && Target && Target->IsInProgress()
		&& FVector::DistSquared(Character->GetActorLocation(), Target->GetOwner()->GetActorLocation()) <= FMath::Square(ReviveRange);

	if (!bValid) StopReviveNearby();
}

bool UReviveComponent::CanBeRevived() const
{
	const AEstivalBaseCharacter* Owner = Cast<AEstivalBaseCharacter>(GetOwner());
	return Owner && !Owner->IsAlive() && RemainingLives > 0 && !bIsInProgress;
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
	if (Owner && Owner->Implements<URevivable>() && IRevivable::Execute_TryRevive(Owner, HealthPercent))
	{
		RemainingLives--;
		OnLivesChanged.Broadcast(RemainingLives);
		OnCompleted.Broadcast();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Revive failed on %s: TryRevive returned false or interface missing"), *GetNameSafe(Owner));
	OnCancelled.Broadcast();
}

bool UReviveComponent::StartReviveNearby()
{
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner());
	if (!Character || !Character->IsAlive() || ReviveTarget.IsValid()) return false;

	UReviveComponent* Target = FindReviveTarget();
	if (!Target || !Target->BeginResurrection(GetOwner())) return false;

	ReviveTarget = Target;
	SetComponentTickEnabled(true);
	return true;
}

void UReviveComponent::StopReviveNearby()
{
	if (UReviveComponent* Target = ReviveTarget.Get()) Target->CancelResurrection();

	ReviveTarget.Reset();
	SetComponentTickEnabled(false);
}

UReviveComponent* UReviveComponent::FindReviveTarget() const
{
	if (!GetWorld()) return nullptr;

	const FVector Origin = GetOwner()->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ReviveRange),
		FCollisionQueryParams(SCENE_QUERY_STAT(FindReviveTarget), false, GetOwner()));

	UReviveComponent* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		UReviveComponent* Candidate = Actor ? Actor->FindComponentByClass<UReviveComponent>() : nullptr;
		if (!Candidate || !Candidate->CanBeRevived()) continue;

		const float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}
	return Best;
}