#include "HoldTask.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Systems/Mike/Combat/Loadouts/AimComponent.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

UHoldTask::UHoldTask()
{
	Lifetime = EAbilityTaskLifetime::KeepAbilityActive;
	bLifetimeEditable = false;
}

void UHoldTask::OnStart_Implementation()
{
	if (!IsInputHeld())
	{
		Release();
		return;
	}

	if (MaxHoldTime > 0.f) GetWorld()->GetTimerManager().SetTimer(ReleaseTimer, this, &UHoldTask::Release, MaxHoldTime, false);

	if (AActor* AimIndicator = GetIndicator(); bAimAtLocation && AimIndicator)
	{
		AimIndicator->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		UpdateIndicator();
	}
}

void UHoldTask::OnInputReleased_Implementation()
{
	Release();
}

void UHoldTask::Release()
{
	if (!IsRunning()) return;

	const FHoldTier* Tier = FindTier(GetElapsedTime());
	UAbilityActivation* Activation = GetActivation();

	if (!Tier)
	{
		if (Activation) Activation->Cancel();
		Finish();
		return;
	}

	if (!CommitAbility()) return;

	FAbilityContext ReleaseContext = GetContext();

	if (bAimAtLocation)
	{
		ReleaseContext.Origin = GetLoadout()->GetAim()->GetAimLocation(MaxAimRange);
		ReleaseContext.bFollowCaster = false;
	}

	if (Activation) Activation->JumpToSection(Tier->MontageSection);

	ExecuteAll(Tier->Effects, ReleaseContext);
	Finish();
}

void UHoldTask::UpdateIndicator() const
{
	Super::UpdateIndicator();

	AActor* AimIndicator = GetIndicator();
	if (bAimAtLocation && IsValid(AimIndicator)) AimIndicator->SetActorLocation(GetLoadout()->GetAim()->GetAimLocation(MaxAimRange));
}

const FHoldTier* UHoldTask::FindTier(const float HeldTime) const
{
	const FHoldTier* Best = nullptr;

	for (const FHoldTier& Tier : Tiers)
	{
		if (HeldTime + UE_KINDA_SMALL_NUMBER < Tier.HoldTime) continue;
		if (!Best || Tier.HoldTime >= Best->HoldTime) Best = &Tier;
	}

	return Best;
}

float UHoldTask::GetProgress_Implementation() const
{
	float FullTime = MaxHoldTime;
	for (const FHoldTier& Tier : Tiers) FullTime = FMath::Max(FullTime, Tier.HoldTime);

	return FullTime > 0.f ? GetElapsedTime() / FullTime : -1.f;
}
