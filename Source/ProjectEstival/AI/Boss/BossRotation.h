#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BossRotation.generated.h"

// boss uses one of these slots, picked at random
USTRUCT(BlueprintType)
struct FBossOpeningStep
{
	GENERATED_BODY()

	// 2 slots = 50/50
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(Categories="Ability."))
	TArray<FGameplayTag> AnyOf;
};

// what the boss does in one phase
USTRUCT(BlueprintType)
struct FBossRotationPhase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation")
	TArray<FBossOpeningStep> Opening;

	// Bigger weight means ability picked more often
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(Categories="Ability."))
	TMap<FGameplayTag, float> Weights;
};

// how far the target must be for an ability
USTRUCT(BlueprintType)
struct FBossAbilityRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin=0, Units="cm"))
	float Min = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin=0, Units="cm"))
	float Max = 100000.f;
};

// The order in which a boss uses its abilities
UCLASS(BlueprintType)
class PROJECTESTIVAL_API UBossRotation : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// missing phases reuse the last one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation")
	TArray<FBossRotationPhase> Phases;

	// the distance each ability needs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(Categories="Ability."))
	TMap<FGameplayTag, FBossAbilityRange> Ranges;

	// it is not part of the rotation, it just fills the gaps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(Categories="Ability."))
	FGameplayTag BasicAttack;

	// basic attacks between two rotation abilities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin=0))
	int32 BasicAttacksBetween = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin=0, Units="s"))
	float MaxTimeBetween = 5.f;

	// if the next ability can't be used this long after it was picked, the boss picks another
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin=0, Units="s"))
	float Patience = 8.f;
};