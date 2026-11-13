#pragma once

#include "CoreMinimal.h"
#include "Abstract/TargetingEffect.h"
#include "Abstract/TargetShape.h"
#include "AreaTargetingEffect.generated.h"

/** Applies its payloads to everything inside a shape at the origin, turned toward the direction. */
UCLASS(meta = (DisplayName = "Area"))
class PROJECTESTIVAL_API UAreaTargetingEffect : public UTargetingEffect
{
	GENERATED_BODY()
	
public:
	virtual TArray<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TObjectPtr<UTargetShape> Shape;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bIncludeInstigator = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDrawDebug = false;
};