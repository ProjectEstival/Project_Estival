#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilityBody.h"
#include "Systems/Mike/Stats/StatTypes.h"
#include "AbilityDefinition.generated.h"

UENUM(BlueprintType)
enum class EAbilityFacing : uint8
{
	None UMETA(DisplayName = "Don't Turn"),
	WholeTime,
	OnCommit
};

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UAbilityDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText DisplayName;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", Units = "s"))
	float Cooldown = 0.5f;
	
	/** How long the ability stays active. 0 = until its montage starts blending out (instant without a montage). Scaled by attack speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", Units = "s"))
	float ActiveDuration = 0.f;

	/** Can start while another ability is active, interrupting it (e.g. dash). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	bool bCanInterrupt = false;

	/** Stat modifiers applied while the ability is active (e.g. slower movement while attacking). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (Categories = "Stats."))
	TMap<FGameplayTag, FStatModifier> ActiveModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (Categories = "Stats.Resources", ClampMin = "0"))
	TMap<FGameplayTag, float> ResourceCosts;
	
	/** Pay the costs and start the cooldown when a Hold is released, instead of when the ability starts. Cancelled or interrupted before that = free. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	bool bCommitOnRelease = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UAnimMontage> Montage;

	/** When the character turns to the aim. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	EAbilityFacing Facing = EAbilityFacing::None;

	/** Activates again when it ends if its button is still held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	bool bRepeatWhileHeld = false;

	/** Once this ability commits (pays its costs), its slot casts this ability instead. Follow-ups ignore and never start the slot cooldown. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UAbilityDefinition> FollowUp;

	/** How long the follow-up stays available after this ability ends. 0 = until used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (ClampMin = "0", Units = "s", EditCondition = "FollowUp != nullptr", EditConditionHides))
	float FollowUpWindow = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityPredicate>> Predicates;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityEffect>> Effects;
};