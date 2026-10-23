#include "DamagePayload.h"

#include "Kismet/GameplayStatics.h"
#include "Systems/Mike/MikeUtils.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

bool UDamagePayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!Target || Target == Context.Instigator) return false;
	
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