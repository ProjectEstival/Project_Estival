#include "SendEventEffect.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void USendEventEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (IsValid(Context.Loadout)) Context.Loadout->SendAbilityEvent(Event);
}
