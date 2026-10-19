#include "DamageEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Mike/MikeUtils.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

void UDamageEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	const float Damage = UMikeUtils::RandomizeFloat(Context.Stats->GetFloat(DamageTag), DamageRandomization) * DamageMultiplier;
	if (Damage <= 0.f) return;
	
	AController* InstigatorController = Context.Instigator->GetInstigatorController();
	TSet<AActor*> AlreadyHit;
 
	for (AActor* Target : GetTargets_Implementation(Context))
	{
		if (!Target || Target == Context.Instigator || AlreadyHit.Contains(Target)) continue;
 
		AlreadyHit.Add(Target);
		UGameplayStatics::ApplyDamage(Target, Damage, InstigatorController, Context.Instigator, DamageType);
	}
}