#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "RuntimeAbilities.generated.h"

class ULoadoutComponent;
class UAnimInstance;

USTRUCT()
struct FAbilitySlot
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	TObjectPtr<UAbilityDefinition> Ability;

	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	TObjectPtr<UAbilityDefinition> FollowUp;

	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	float CooldownStart = 0;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	float CooldownEnd = 0;

	FTimerHandle FollowUpTimer;
	UAbilityDefinition* GetCurrent() const { return FollowUp ? FollowUp.Get() : Ability.Get(); }
};

/**
 * One use of an ability, from activation until it ends. The loadout creates one each time an ability starts.
 */
UCLASS(Transient)
class PROJECTESTIVAL_API UAbilityActivation : public UObject
{
	GENERATED_BODY()

public:
	void Start(ULoadoutComponent* InLoadout, FGameplayTag InSlot, UAbilityDefinition* InAbility, bool bInFromFollowUp);
	bool Commit();
	void Cancel();
	void TryEnd();

	void JumpToSection(FName Section) const;
	bool IsActive() const { return bActive; }
	bool IsCommitted() const { return bCommitted; }
	FGameplayTag GetSlot() const { return Slot; }
	UAbilityDefinition* GetAbility() const { return Ability; }
	FSimpleMulticastDelegate OnInterrupted;

private:
	void PlayMontage(float PlayRate);
	void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void ReleaseMontageLoop() const;
	void TimeUp();
	void End(bool bInterrupted);
	UAnimInstance* GetAnimInstance() const;

	UPROPERTY()
	TObjectPtr<ULoadoutComponent> Loadout;

	UPROPERTY()
	TObjectPtr<UAbilityDefinition> Ability;

	FGameplayTag Slot;
	FTimerHandle DurationTimer;
	bool bActive = false;
	bool bCommitted = false;
	bool bFromFollowUp = false;
	bool bMontagePlaying = false;
	bool bTimeUp = false;
};