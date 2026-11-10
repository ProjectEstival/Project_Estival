#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Systems/Core/TagDefinitions.h"
#include "AbilityEventNotify.generated.h"

/** Sends an ability event from the owner's loadout at this frame (hit frame, release frame). */
UCLASS(meta = (DisplayName = "Ability Event"))
class PROJECTESTIVAL_API UAbilityEventNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (Categories = "AbilityEvent."))
	FGameplayTag Event = TagDefinitions::AbilityEvent_Hit;
};