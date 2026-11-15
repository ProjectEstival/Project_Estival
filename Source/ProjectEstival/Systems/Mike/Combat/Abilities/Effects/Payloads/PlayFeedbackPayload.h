#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "PlayFeedbackPayload.generated.h"

class UCameraShakeBase;
class UFMODEvent;
class UNiagaraSystem;

/** Plays a visual, a sound and a camera shake. Fill in only the ones you want. */
UCLASS(meta = (DisplayName = "Play Feedback"))
class PROJECTESTIVAL_API UPlayFeedbackPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> Visual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UFMODEvent> Sound;

	/** Shakes the camera the players share, wherever this plays. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0, EditCondition = "CameraShake != nullptr", EditConditionHides))
	float ShakeStrength = 1.f;

	/** Play at the ability's Origin instead of on the receiver */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	bool bAtOrigin = false;

	/** A bone or socket on the receiver, such as a hand or a muzzle. Empty plays at its center. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (EditCondition = "!bAtOrigin", EditConditionHides))
	FName Socket;

	/** Move along with the receiver instead of staying where it started. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (EditCondition = "!bAtOrigin", EditConditionHides))
	bool bAttach = false;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;

private:
	USceneComponent* FindPart(const AActor* Receiver) const;
};
