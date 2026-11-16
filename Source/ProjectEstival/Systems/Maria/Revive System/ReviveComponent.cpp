#include "ReviveComponent.h"
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

bool UReviveComponent::TryInteract_Implementation(AActor* Interactor)
{
	AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner());
	if (!Character || !Character->TryRevive(HealthPercent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Revive failed on %s"), *GetNameSafe(GetOwner()));
		OnCancelled.Broadcast();
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Revived! on %s"), *GetNameSafe(GetOwner()));
	RemainingLives--;
	OnLivesChanged.Broadcast(RemainingLives);
	OnCompleted.Broadcast();
	return true;
}
#pragma endregion

#pragma region Target Side
bool UReviveComponent::CanBeRevived() const
{
	const AEstivalBaseCharacter* Owner = Cast<AEstivalBaseCharacter>(GetOwner());
	return Owner && !Owner->IsAlive() && RemainingLives > 0;
}
#pragma endregion