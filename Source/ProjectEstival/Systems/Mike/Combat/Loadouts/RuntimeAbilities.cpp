#include "RuntimeAbilities.h"
#include "LoadoutComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

void UAbilityActivation::Start(ULoadoutComponent* InLoadout, const FGameplayTag InSlot, UAbilityDefinition* InAbility, const bool bInFromFollowUp)
{
	Loadout = InLoadout;
	Slot = InSlot;
	Ability = InAbility;
	bFromFollowUp = bInFromFollowUp;
	bActive = true;

	if (!Ability->bCommitOnRelease && !Commit()) return;

	const float AttackSpeed = Loadout->GetAttackSpeed();
	if (UStatisticsComponent* Stats = Loadout->GetStats()) Stats->AddModifierSet(Ability->ActiveModifiers, this);
	PlayMontage(AttackSpeed);

	UAbilityEffect::ExecuteAll(Ability->Effects, Loadout->MakeContext(Ability, this));

	if (!bActive) return;

	if (Ability->ActiveDuration > 0.f)
	{
		Loadout->GetWorld()->GetTimerManager().SetTimer(DurationTimer, this, &UAbilityActivation::TimeUp, Ability->ActiveDuration / AttackSpeed, false);
	}
	else if (!bMontagePlaying)
	{
		bTimeUp = true;
	}

	TryEnd();
}

bool UAbilityActivation::Commit()
{
	if (bCommitted) return true;
	if (!bActive) return false;

	if (!Loadout->TryCommit(Slot, Ability, bFromFollowUp))
	{
		Cancel();
		return false;
	}

	bCommitted = true;
	Loadout->OnAbilityCommitted.Broadcast(Slot, Ability);
	return bActive;
}

void UAbilityActivation::Cancel()
{
	End(true);
}

void UAbilityActivation::TryEnd()
{
	if (!bActive || Loadout->HasTaskKeepingActive(this)) return;
	if (bMontagePlaying && Ability->ActiveDuration <= 0.f) ReleaseMontageLoop();
	if (bTimeUp) End(false);
}

void UAbilityActivation::JumpToSection(const FName Section) const
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (bMontagePlaying && AnimInstance && !Section.IsNone()) AnimInstance->Montage_JumpToSection(Section, Ability->Montage);
}

void UAbilityActivation::PlayMontage(const float PlayRate)
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!Ability->Montage || !AnimInstance) return;
	if (AnimInstance->Montage_Play(Ability->Montage, PlayRate) <= 0.f) return;

	bMontagePlaying = true;

	FOnMontageBlendingOutStarted BlendingOut = FOnMontageBlendingOutStarted::CreateUObject(this, &UAbilityActivation::HandleMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOut, Ability->Montage);
}

void UAbilityActivation::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bActive) return;

	bMontagePlaying = false;
	if (Ability->ActiveDuration > 0.f) return;

	bTimeUp = true;
	TryEnd();
}

void UAbilityActivation::ReleaseMontageLoop() const
{
	const UAnimInstance* AnimInstance = GetAnimInstance();
	FAnimMontageInstance* MontageInstance = AnimInstance ? AnimInstance->GetActiveInstanceForMontage(Ability->Montage) : nullptr;
	if (!MontageInstance) return;

	const int32 Section = Ability->Montage->GetSectionIndex(MontageInstance->GetCurrentSection());
	if (Section != INDEX_NONE && MontageInstance->GetNextSectionID(Section) == Section) MontageInstance->SetNextSectionID(Section, INDEX_NONE);
}

void UAbilityActivation::TimeUp()
{
	bTimeUp = true;
	TryEnd();
}

void UAbilityActivation::End(const bool bInterrupted)
{
	if (!bActive) return;
	bActive = false;

	if (!bInterrupted && !bCommitted && Loadout->TryCommit(Slot, Ability, bFromFollowUp))
	{
		bCommitted = true;
		Loadout->OnAbilityCommitted.Broadcast(Slot, Ability);
	}

	if (bInterrupted && bMontagePlaying)
	{
		if (ACharacter* Character = Cast<ACharacter>(Loadout->GetOwner())) Character->StopAnimMontage(Ability->Montage);
	}

	if (const UWorld* World = Loadout->GetWorld()) World->GetTimerManager().ClearTimer(DurationTimer);
	if (UStatisticsComponent* Stats = Loadout->GetStats()) Stats->RemoveAllFromSource(this);

	if (bInterrupted) OnInterrupted.Broadcast();

	Loadout->CancelTasks(this);
	Loadout->HandleActivationEnded(this, bInterrupted);
}

UAnimInstance* UAbilityActivation::GetAnimInstance() const
{
	const ACharacter* Character = Cast<ACharacter>(Loadout->GetOwner());
	return Character && Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
}
