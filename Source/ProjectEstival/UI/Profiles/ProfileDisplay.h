#pragma once

#include "Components/HorizontalBox.h"
#include "CoreMinimal.h"

#include "ProfileDisplay.generated.h"

class UProfileDisplayEntry;
class UPlayerProfileSubsystem;
class ULocalPlayer;

// Horizontal list of assigned player profiles, ordered by their stable color slot
UCLASS()
class PROJECTESTIVAL_API UProfileDisplay : public UHorizontalBox
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Main Menu|Ready Players")
	void RefreshDisplay();
	
protected:
	virtual void OnWidgetRebuilt() override;
	virtual void BeginDestroy() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Ready Players")
	TSubclassOf<UProfileDisplayEntry> PlayerReadyEntryWidgetClass;
	
private:
	void BindProfileEvents();
	void UnbindProfileEvents();
	
	UFUNCTION()
	void HandleProfileChanged(ULocalPlayer* LocalPlayer);
	
	UFUNCTION()
	void HandleProfileAssigned(ULocalPlayer* LocalPlayer, FGuid ProfileId);
	
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileSubsystem> BoundProfileSubsystem;
};
