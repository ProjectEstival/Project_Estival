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
	
	UPROPERTY(EditDefaultsOnly, Category = "Onboarding")
	TSubclassOf<UProfileSelectionPopup> ProfilePopupClass;
private:
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileSubsystem> PlayerSubsystem;
	
	UPROPERTY(Transient)
	TObjectPtr<UProfileSelectionPopup> ActivePopup;
	
	UFUNCTION()
	void HandleOnboardingPlayerChanged(ULocalPlayer* LocalPlayer);
	UFUNCTION()
	void HandleProfileSelected(ULocalPlayer* LocalPlayer, FGuid ProfileId);
	UFUNCTION()
	void HandleProfileSelectionCancelled(ULocalPlayer* LocalPlayer);
};