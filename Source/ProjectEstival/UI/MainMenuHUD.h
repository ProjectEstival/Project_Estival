#pragma once

#include "Systems/Core/SaveSystem/PlayerProfileSubsystem.h"
#include "CoreMinimal.h"
#include "HUDBase.h"
#include "MainMenuHUD.generated.h"

class UProfileSelectionPopup;
class ULocalPlayer;

UCLASS()
class PROJECTESTIVAL_API AMainMenuHUD : public AHUDBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Player popup
	UPROPERTY(EditDefaultsOnly, Category = "Onboarding")
	TSubclassOf<UProfileSelectionPopup> ProfilePopupClass;
private:
	// Object references
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileSubsystem> PlayerProfileSubsystem;
	UPROPERTY(Transient)
	TObjectPtr<UProfileSelectionPopup> ActivePopup;
	
	// Player popup
	UFUNCTION()
	void HandleOnboardingPlayerChanged(ULocalPlayer* LocalPlayer);
	UFUNCTION()
	void HandleProfileSelected(ULocalPlayer* LocalPlayer, FGuid ProfileId);
	UFUNCTION()
	void HandleProfileSelectionCancelled(ULocalPlayer* LocalPlayer);
};