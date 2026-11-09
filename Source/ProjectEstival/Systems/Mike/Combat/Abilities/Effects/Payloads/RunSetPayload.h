#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "RunSetPayload.generated.h"

/** A list of payloads kept in one asset, so several abilities can share them and be balanced in one place. */
UCLASS(BlueprintType)
class PROJECTESTIVAL_API UPayloadSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Payload Set")
	TArray<TObjectPtr<UAbilityEffectPayload>> Payloads;
};

/** Applies every payload from a Payload Set to the receiver. */
UCLASS(meta = (DisplayName = "Run Payload Set"))
class PROJECTESTIVAL_API URunSetPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Payload Set")
	TObjectPtr<UPayloadSet> Set;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override
	{
		return Set && ApplyAll(Set->Payloads, Context, Target);
	}
};
