#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "TargetShape.generated.h"

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UTargetShape : public UObject
{
	GENERATED_BODY()
	
public:
	void CollectActors(const UWorld* World, const FVector& Center, const FQuat& Rotation, ECollisionChannel Channel,
		const FCollisionQueryParams& Params, TArray<AActor*>& OutActors) const;
	
	virtual void DrawDebug(const UWorld* World, const FVector& Center, const FQuat& Rotation, float Duration) const {}
	
protected:
	virtual FCollisionShape GetCollisionShape() const { return FCollisionShape(); }
	virtual FVector GetLocalOffset() const { return FVector::ZeroVector; }
	virtual bool IsInside(const AActor* Actor, const FVector& Center, const FQuat& Rotation) const { return true; }
};