#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CompanionTactics.generated.h"

class UEnvQuery;
class UTexture2D;

// One row of the ability list: use this slot when the enemy is between MinRange and MaxRange 
USTRUCT(BlueprintType)
struct FCompanionAbilityRule
{
	GENERATED_BODY()

	// which loadout slot to use, for example Ability.Dash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability", meta=(Categories="Ability."))
	FGameplayTag Slot;

	// the enemy is closer than this? skip this row
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability", meta=(ClampMin=0, Units="cm"))
	float MinRange = 0.f;

	// the enemy is farther than this? skip this row
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability", meta=(ClampMin=0, Units="cm"))
	float MaxRange = 200.f;
};

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UCompanionTactics : public UDataAsset
{
	GENERATED_BODY()

public:
	// what a menu or the HUD shows. These are the same three fields like Mike's UAbilityDefinition
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info", meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info")
	TObjectPtr<UTexture2D> Icon;

	// tried from the top. The first row that is in range and fires wins
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Abilities")
	TArray<FCompanionAbilityRule> Abilities;

	// scores the enemies in the fight to pick a new target. Empty = the nearest one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
	TObjectPtr<UEnvQuery> TargetQuery;

	// where to stand around the enemy: the formation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Formation")
	TObjectPtr<UEnvQuery> AttackSpotQuery;
	
	// color of this style's debug line to the attack target
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor = FColor::Red;
};