#include "ReviveComponent.h"
#include "Revivable.h"
#include "Systems/Core/Characters/CharacterBase.h"

#pragma region Lifecycle
UReviveComponent::UReviveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UReviveComponent::BeginPlay()
{
	Super::BeginPlay();
	RemainingLives = FMath::Max(0, MaxLives);
	OnLivesChanged.Broadcast(RemainingLives);
}
#pragma endregion

#pragma region Interactable
bool UReviveComponent::CanInteract_Implementation(AActor* Interactor) const { return CanBeRevived(); }

void UReviveComponent::Interact_Implementation(AActor* Interactor)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<URevivable>() && IRevivable::Execute_TryRevive(Owner, HealthPercent))
	{
		RemainingLives--;
		OnLivesChanged.Broadcast(RemainingLives);
		OnCompleted.Broadcast();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Revive failed on %s"), *GetNameSafe(Owner));
	OnCancelled.Broadcast();
}
#pragma endregion

#pragma region Target Side
bool UReviveComponent::CanBeRevived() const
{
	const AEstivalBaseCharacter* Owner = Cast<AEstivalBaseCharacter>(GetOwner());
	return Owner && !Owner->IsAlive() && RemainingLives > 0;
}
#pragma endregion