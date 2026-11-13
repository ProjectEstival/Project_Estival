#include "SpawnProjectilesEffect.h"
#include "Systems/Mike/Combat/Abilities/Actors/AbilityProjectile.h"

USpawnProjectilesEffect::USpawnProjectilesEffect()
{
	SpawnOffset = FVector::ForwardVector * 50.f;
}

void USpawnProjectilesEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (!Context.World || !ProjectileClass) return;
	
	const FVector Forward = Context.Direction.IsNearlyZero() ? FVector::ForwardVector : Context.Direction;
	const FVector SpawnLocation = GetSpawnTransform(Context).GetLocation();

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

	FAbilityContext ProjectileContext = Context;
	ProjectileContext.Direction = Direction;

	AAbilityActor* Actor = BeginSpawn(ProjectileContext, ProjectileClass, SpawnTransform);
	if (AAbilityProjectile* Projectile = Cast<AAbilityProjectile>(Actor)) Projectile->SetupProjectile(Speed, Range);

	FinishSpawn(ProjectileContext, Actor, SpawnTransform);
}
