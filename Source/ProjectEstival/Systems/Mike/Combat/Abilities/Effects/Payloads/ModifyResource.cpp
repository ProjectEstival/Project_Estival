#include "ModifyResource.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

bool UModifyResource::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (ResourceChanges.Num() == 0) return true;
	
	const AEstivalBaseCharacter* TargetCharacter = Cast<AEstivalBaseCharacter>(Target);
	if (!TargetCharacter || !TargetCharacter->IsAlive()) return false;
	
	float TotalApplied = 0;
	
	for (const auto& Change : ResourceChanges)
	{
		if (Change.bFromStat && !Context.Stats) continue;
		
		const float Amount = Change.bFromStat? Context.Stats->GetFloat(Change.AmountStat) * Change.Multiplier : Change.Amount;
		if (Amount <= 0) continue;

		const float Applied = TargetCharacter->GetResources()->ModifyResource(Change.ModifyResource, Change.Deplete? -Amount : Amount);
		TotalApplied += FMath::Abs(Applied);
	}
	
	return TotalApplied > 0;
}