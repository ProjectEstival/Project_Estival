#pragma once

#include "CoreMinimal.h"
#include "AbilityTask.h"
#include "MoveCasterTask.generated.h"

class ACharacter;

UENUM(BlueprintType)
enum class EMoveCasterDirection : uint8
{
	/** Along the ability's direction */
	Aim,
	/** The way the caster is moving. Where they face when standing still. */
	Movement,
	/** Where the caster faces. */
	Facing,
	/** To the ability's Origin */
	ToOrigin
};

/** Moves the caster along the ground over a set time, or teleports them if the time is 0. Interrupting the ability stops the move. */
UCLASS(meta = (DisplayName = "Move Caster"), HideFunctions = (OnStart, OnEnd))
class PROJECTESTIVAL_API UMoveCasterTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	UMoveCasterTask();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move")
	EMoveCasterDirection Direction = EMoveCasterDirection::Aim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move", meta = (ClampMin = 0, Units = "cm", EditCondition = "Direction != EMoveCasterDirection::ToOrigin", EditConditionHides))
	float Distance = 1000.f;

	/** How long the move takes. 0 teleports. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move", meta = (ClampMin = 0, Units = "s"))
	float Duration = 0.2f;

	/** Teleport past walls instead of stopping in front of them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move", meta = (EditCondition = "Duration == 0", EditConditionHides))
	bool bThroughWalls = false;

	/** Run where the caster ends up, unless the move was interrupted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move")
	TArray<TObjectPtr<UAbilityEffect>> ArriveEffects;

	virtual void OnStart_Implementation() override;
	virtual void OnEnd_Implementation(bool bCancelled) override;

private:
	FVector FindDestination(const FAbilityContext& Now) const;
	void Teleport(AActor* Caster, FVector To) const;
	void StartMoving(const ACharacter* Character, const FVector& To);
	void Arrive();

	uint16 MoveId = 0;
};
