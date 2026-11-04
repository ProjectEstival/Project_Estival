#include "SpawnProjectilesEffect.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Systems/Mike/Combat/Abilities/Actors/AbilityProjectile.h"

void USpawnProjectilesEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (!Context.World || !ProjectileClass) return;
	
	const FVector Forward = Context.Direction.IsNearlyZero() ? FVector::ForwardVector : Context.Direction;
	const FVector SpawnLocation = Context.Origin + Forward.ToOrientationQuat().RotateVector(SpawnOffset);
	
	const bool bFullCircle = SpreadAngle >= 360.f;
	const float Step = ProjectileCount > 1 ? SpreadAngle / (bFullCircle ? ProjectileCount : ProjectileCount - 1) : 0.f;
	const float FirstAngle = ProjectileCount > 1 ? -SpreadAngle * 0.5f : 0.f;
	
	for (int32 Index = 0; Index < ProjectileCount; ++Index)
	{
		const FVector Direction = Forward.RotateAngleAxis(FirstAngle + Step * Index, FVector::UpVector);
		SpawnProjectile(Context, SpawnLocation, Direction);
	}
}

void USpawnProjectilesEffect::SpawnProjectile(const FAbilityContext& Context, const FVector& Location, const FVector& Direction) const
{
	const FTransform SpawnTransform(Direction.Rotation(), Location);
	
	AAbilityProjectile* Projectile = Context.World->SpawnActorDeferred<AAbilityProjectile>(
		ProjectileClass,
		SpawnTransform,
		Context.Instigator,
		Cast<APawn>(Context.Instigator),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	if (!Projectile) return;
	
	FAbilityContext ProjectileContext = Context;
	ProjectileContext.Direction = Direction;
	
	Projectile->Initialize(ProjectileContext, this, Speed, Range);
	Projectile->FinishSpawning(SpawnTransform);
}
