#pragma once

#include "CoreMinimal.h"
#include "AbilityTask.h"
#include "HoldTask.generated.h"

/** What happens on release when the button was held at least HoldTime. */
USTRUCT(BlueprintType)
struct FHoldTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier", meta = (ClampMin = 0, Units = "s"))
	float HoldTime = 0.f;

	/** Montage section to jump to on release (e.g. "Release"). None = the montage keeps playing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	FName MontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	TArray<TObjectPtr<UAbilityEffect>> Effects;
};

/**
 * Waits while the ability's button is held (charge, aim), then runs the highest tier reached on release.
 * Released before any tier: the ability cancels.
 */
UCLASS(meta = (DisplayName = "Hold (Charge / Aim)"), HideFunctions = (OnStart, OnInputReleased))
class PROJECTESTIVAL_API UHoldTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	UHoldTask();
	virtual float GetProgress_Implementation() const override;

protected:
	/** While held, the indicator sits on the aimed point. On release the effects happen there. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hold")
	bool bAimAtLocation = false;

	/** How far from the caster the aim can be. 0 = no limit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hold", meta = (ClampMin = 0, Units = "cm", EditCondition = "bAimAtLocation", EditConditionHides))
	float MaxAimRange = 1000.f;

	/** Releases automatically after this long. 0 = no limit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hold", meta = (ClampMin = 0, Units = "s"))
	float MaxHoldTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hold", meta = (TitleProperty = "At {HoldTime}s Hold | Montage Section: {MontageSection}"))
	TArray<FHoldTier> Tiers;

	virtual void OnStart_Implementation() override;
	virtual void OnInputReleased_Implementation() override;
	virtual void UpdateIndicator() const override;

private:
	void Release();
	const FHoldTier* FindTier(float HeldTime) const;

	FTimerHandle ReleaseTimer;
};
