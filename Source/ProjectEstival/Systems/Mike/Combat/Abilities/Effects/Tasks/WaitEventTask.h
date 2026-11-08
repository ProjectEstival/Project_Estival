#pragma once

#include "CoreMinimal.h"
#include "AbilityTask.h"
#include "Systems/Core/TagDefinitions.h"
#include "WaitEventTask.generated.h"

/** Runs its effects when the ability receives an event, usually an Ability Event notify in the montage (hit frame). Stops when the ability ends. */
UCLASS(meta = (DisplayName = "Wait Event"))
class PROJECTESTIVAL_API UWaitEventTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	UWaitEventTask();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wait Event", meta = (Categories = "AbilityEvent."))
	FGameplayTag Event = TagDefinitions::AbilityEvent_Hit;

	/** Stop after the first event. Off = run every time the event arrives (multi-hit montages). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wait Event")
	bool bOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wait Event")
	TArray<TObjectPtr<UAbilityEffect>> Effects;

	virtual void OnAbilityEvent_Implementation(FGameplayTag ReceivedEvent) override;
};
