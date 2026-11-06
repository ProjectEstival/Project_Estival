#include "DamagePayload.h"

#include "Kismet/GameplayStatics.h"
#include "Systems/Mike/MikeUtils.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

bool UDamagePayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target) || !IsValid(Context.Instigator) || !Context.Stats) return false;
	if (!Target->CanBeDamaged()) return false;
	
	const float Damage = UMikeUtils::RandomizeFloat(Context.Stats->GetFloat(DamageTag), DamageRandomization) * DamageMultiplier;
	if (Damage <= 0.f) return false;
	
	AController* InstigatorController = Context.Instigator->GetInstigatorController();
	const float DamageApplied = UGameplayStatics::ApplyDamage(
		Target, 
		Damage,
		InstigatorController, 
		Context.Instigator, 
		DamageType);
	
	return !FMath::IsNearlyZero(DamageApplied);
}