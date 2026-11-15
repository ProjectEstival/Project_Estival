#include "AbilityBody.h"
#include "Engine/Engine.h"

UWorld* UAbilityObject::GetWorld() const
{
	return GEngine && !HasAnyFlags(RF_ClassDefaultObject) ? GEngine->GetCurrentPlayWorld() : nullptr;
}
