#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AimComponent.generated.h"

/** Where a character is aiming. */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UAimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAimComponent();

	/** Aim in a direction, e.g. the right stick. */
	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetAimDirection(FVector Direction);

	/** Aim at a world location, e.g. an AI's target. */
	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetAimLocation(FVector Location);

	/** Aim at what is under the mouse cursor, read every time an ability needs it. */
	UFUNCTION(BlueprintCallable, Category = "Aim")
	void AimAtCursor();

	/** Stop aiming: abilities go where the character faces. */
	UFUNCTION(BlueprintCallable, Category = "Aim")
	void ClearAim();

	/** The flat direction abilities go in: toward the aim, or where the character faces. */
	UFUNCTION(BlueprintPure, Category = "Aim")
	FVector GetAimDirection() const;

	/** Where location abilities land, at most MaxRange away (0 = no limit). Direction aim lands MaxRange away along it. */
	UFUNCTION(BlueprintPure, Category = "Aim")
	FVector GetAimLocation(float MaxRange = 0.f) const;

private:
	enum class EAimMode : uint8 { None, Direction, Location, Cursor };

	/** The aimed world point, for location and cursor aim. */
	bool FindAimPoint(FVector& OutPoint) const;

	EAimMode AimMode = EAimMode::None;
	FVector Aim = FVector::ZeroVector;
};
