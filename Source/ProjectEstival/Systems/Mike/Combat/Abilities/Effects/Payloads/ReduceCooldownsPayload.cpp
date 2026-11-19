#include "ReduceCooldownsPayload.h"
#include "Systems/Core/Characters/CharacterBase.h"

bool UReduceCooldownsPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(Target);
	if (!Character) return false;
	
	bool bReducedAny = false;
	ULoadoutComponent* Loadout = Character->GetLoadout();
	
	for (const auto& Reduction : ReduceSeconds)
	{
		if (!Loadout->IsOnCooldown(Reduction.Key)) continue;
		Loadout->ReduceCooldown(Reduction.Key, Reduction.Value);
		bReducedAny = true;
	}

	for (const auto& Reduction : ReducePercent)
	{
		if (!Loadout->IsOnCooldown(Reduction.Key)) continue;
		
		const float Seconds = Loadout->GetAbility(Reduction.Key)->Cooldown * Reduction.Value / 100.f;
		if (Seconds <= 0.f) continue;
		Loadout->ReduceCooldown(Reduction.Key, Seconds);
		bReducedAny = true;
	}
	return bReducedAny;
}
