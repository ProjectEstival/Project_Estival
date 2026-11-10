#include "AbilityEventNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void UAbilityEventNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const AEstivalBaseCharacter* Owner = MeshComp ? Cast<AEstivalBaseCharacter>(MeshComp->GetOwner()) : nullptr;
	if (ULoadoutComponent* Loadout = Owner ? Owner->GetLoadout() : nullptr) Loadout->SendAbilityEvent(Event);
}

FString UAbilityEventNotify::GetNotifyName_Implementation() const
{
	return Event.IsValid() ? Event.GetTagName().ToString() : Super::GetNotifyName_Implementation();
}
