#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "ProfileSelectionPopup.generated.h"

class UButton;
class UScrollBox;
class ULocalPlayer;
class UPlayerProfileCard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateProfileRequested, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileSelected, ULocalPlayer*, LocalPlayer, FGuid, ProfileId);
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
	void ConfirmSelection(const FGuid& ProfileId);
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void CancelSelection();
	void CompleteSelection();
	
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FOnCreateProfileRequested OnCreateProfileRequested;
	
	void SetTargetLocalPlayer(ULocalPlayer* LocalPlayer);
	ULocalPlayer* GetTargetLocalPlayer() const { return TargetLocalPlayer; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	bool bPendingInitialFocus = false;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ProfileScrollBox;
	UPROPERTY(EditDefaultsOnly, Category = "Profile Selection")
	TSubclassOf<UPlayerProfileCard> ProfileCardClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Profile Selection")
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;
		
	// Receives the selected profile's stable ID.
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile Selection")
	void OnSelectionConfirmed(FGuid ProfileId);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile Selection")
	void OnSelectionCancelled();
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPlayerProfileCard>> SpawnedCards;
	
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileCard> PendingNameCard;
	
	UPROPERTY(Transient)
	bool bSelectionCompleted = false;
	
	void RebuildProfileCards();
	bool FocusFirstCard();
	
	UFUNCTION()
	void HandleCardClicked(UPlayerProfileCard* Card);
	
	UFUNCTION()
	void HandleNameCommitted(UPlayerProfileCard* Card, FText EnteredName);
	void BeginCreatingProfile();
	bool bPendingNameFocus = false;

	static UScrollBoxSlot* InsertWidgetAt(UScrollBox* ScrollBox, UWidget* NewWidget, int32 Index);
};
