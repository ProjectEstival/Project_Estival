#pragma once

#include "UObject/Object.h"
#include "CoreMinimal.h"

#include "PlayerOnboardingQueue.generated.h"

class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnboardingQueuePlayerChanged, ULocalPlayer*, LocalPlayer);

// Orchestrates profile onboarding
UCLASS()
class PROJECTESTIVAL_API UPlayerOnboardingQueue : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Onboarding")
	FOnboardingQueuePlayerChanged OnCurrentPlayerChanged;
	
	void SetMandatoryPlayer(ULocalPlayer* LocalPlayer);
	bool IsMandatoryPlayer(const ULocalPlayer* LocalPlayer) const;
	void Enqueue(ULocalPlayer* LocalPlayer);
	void CompleteCurrent(ULocalPlayer* LocalPlayer);
	void CancelCurrent(ULocalPlayer* LocalPlayer);
	void RemovePlayer(ULocalPlayer* LocalPlayer);
	void Continue();
	ULocalPlayer* GetCurrentPlayer() const { return CurrentPlayer; }
	
private:
	UPROPERTY()
	TArray<TObjectPtr<ULocalPlayer>> PendingPlayers;
	UPROPERTY()
	TObjectPtr<ULocalPlayer> CurrentPlayer;
	TWeakObjectPtr<ULocalPlayer> MandatoryPlayer;
	void AdvanceIfIdle();
};
