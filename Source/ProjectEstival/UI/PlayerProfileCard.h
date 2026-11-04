#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "PlayerProfileCard.generated.h"

class UButton;
class UEditableText;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerProfileCardClicked, class UPlayerProfileCard*, Card);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerProfileNameCommitted, UPlayerProfileCard*, Card, FText, EnteredName);

UCLASS()
class PROJECTESTIVAL_API UPlayerProfileCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Events
	FOnPlayerProfileCardClicked OnCardClicked;
	FOnPlayerProfileNameCommitted OnNameCommitted;
	
	// Initialization
	void SetProfile(const FGuid& InProfileId, const FString& InProfileName);
	FGuid GetProfileId() const { return ProfileId; }
	void SetCreateCard();
	bool IsCreateCard() const { return bIsCreateCard; }
	void FocusCard(APlayerController* PlayerController) const;
	
	// Naming
	void BeginNameEntry();
	void RejectNameEntry() const;
	bool IsNamingProfile() const { return bIsNamingProfile; }
	void FocusNameEntry() const;
	
protected:
	// Initialization
	virtual void NativeOnInitialized() override;
	UPROPERTY(Transient)
	FGuid ProfileId;
	UPROPERTY(Transient)
	bool bIsCreateCard = false;
	
	// Object references
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CardButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ProfileNameText;
	
private:
	UFUNCTION()
	void HandleCardClicked();
	
	// Naming
	UPROPERTY(Transient)
	bool bIsNamingProfile = false;
	UFUNCTION()
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};