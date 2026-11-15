#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "PlayerProfileCard.generated.h"

class APlayerController;
class UEditableText;
class ULocalPlayer;
class UWidget;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerProfileCardClicked, class UPlayerProfileCard*, Card);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerProfileNameCommitted, UPlayerProfileCard*, Card, FText, EnteredName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileCardLongPressed, UPlayerProfileCard*, Card);

UCLASS()
class PROJECTESTIVAL_API UPlayerProfileCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Events
	FOnPlayerProfileCardClicked OnCardClicked;
	FOnPlayerProfileNameCommitted OnNameCommitted;
	UPROPERTY(BlueprintAssignable)
	FOnProfileCardLongPressed OnCardLongPressed;
	
	// Initialization
	void SetProfile(const FGuid& InProfileId, const FString& InProfileName);
	FGuid GetProfileId() const { return ProfileId; }
	void SetCreateCard();
	bool IsCreateCard() const { return bIsCreateCard; }
	void FocusCard(APlayerController* PlayerController) const;
	
	// Naming
	void BeginNameEntry();
	void RejectNameEntry() const;
	void CancelNameEdit();
	bool IsNamingProfile() const { return bIsNamingProfile; }
	void FocusNameEntry(APlayerController* PlayerController) const;
	void BeginNameEdit();
	bool CommitNameEntry();
	void EndNameEntry();
	
protected:
	// Initialization
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	UPROPERTY(Transient)
	FGuid ProfileId;
	UPROPERTY(Transient)
	bool bIsCreateCard = false;
	
	// Object references
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CardButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ProfileNameText;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float LongPressTimerInterval = 0.6f;
	
private:
	// Input
	UFUNCTION()
	void HandleCardPressed();
	UFUNCTION()
	void HandleCardReleased();
	UFUNCTION()
	void HandleCardClicked();
	void HandleLongPressTriggered();
	bool bLongPressTriggered = false;
	FTimerHandle LongPressTimerHandle;
	
	// Naming
	UPROPERTY(Transient)
	bool bIsNamingProfile = false;
	UPROPERTY(Transient)
	FText OriginalProfileName;
	TWeakObjectPtr<ULocalPlayer> KeyboardFocusPlayer;
	TWeakObjectPtr<UWidget> KeyboardPreviousFocusWidget;
	bool bKeyboardFocusCaptured = false;
	void RestoreKeyboardTextFocus();
	UFUNCTION()
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
