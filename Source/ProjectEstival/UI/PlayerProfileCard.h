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
	UPROPERTY(BlueprintAssignable, Category = "Profile Card")
	FOnPlayerProfileCardClicked OnCardClicked;
	
	UPROPERTY(BlueprintAssignable, Category = "Profile Card")
	FOnPlayerProfileNameCommitted OnNameCommitted;
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void SetProfile(const FGuid& InProfileId, const FString& InProfileName);
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void SetCreateCard();
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void BeginNameEntry();
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void RejectNameEntry();
	
	UFUNCTION(BlueprintPure, Category = "Profile Card")
	bool IsNamingProfile() const { return bIsNamingProfile; }
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void FocusCard(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintPure, Category = "Profile Card")
	bool IsCreateCard() const { return bIsCreateCard; }
	
	UFUNCTION(BlueprintPure, Category = "Profile Card")
	FGuid GetProfileId() const { return ProfileId; }
	
	UFUNCTION(BlueprintCallable, Category = "Profile Card")
	void FocusNameEntry();
	
protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CardButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ProfileNameText;
	
private:
	UPROPERTY(Transient)
	FGuid ProfileId;
	
	UPROPERTY(Transient)
	bool bIsCreateCard = false;
	
	UPROPERTY(Transient)
	bool bIsNamingProfile = false;
	
	UFUNCTION()
	void HandleCardClicked();
	
	UFUNCTION()
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};