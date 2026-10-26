#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "ProfileSelectionPopup.generated.h"

class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileSelected, ULocalPlayer*, LocalPlayer, int32, ProfileIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileSelectionCancelled, ULocalPlayer*, LocalPlayer);

UCLASS()
class PROJECTESTIVAL_API UProfileSelectionPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FOnProfileSelected OnProfileSelected;
	
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FOnProfileSelectionCancelled OnProfileSelectionCancelled;
	
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void ConfirmSelection(int32 ProfileIndex);
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void CancelSelection();
	
	void SetTargetLocalPlayer(ULocalPlayer* LocalPlayer);
	ULocalPlayer* GetTargetLocalPlayer() const { return TargetLocalPlayer; }
	
protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Profile Selection")
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile Selection")
	void OnTargetPlayerAssigned(int32 PlayerIndex);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile Selection")
	void OnSelectionConfirmed(int32 ProfileIndex);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile Selection")
	void OnSelectionCancelled();
	
private:
	bool bSelectionCompleted = false;
};
