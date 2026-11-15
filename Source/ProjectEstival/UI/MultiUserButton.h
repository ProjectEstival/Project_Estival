#pragma once

#include "Components/Button.h"
#include "CoreMinimal.h"

#include "MultiUserButton.generated.h"

class APlayerController;
class ULocalPlayer;
class SMultiUserButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiUserButtonEvent, APlayerController*, Instigator);

// UButton variant that preserves the Slate user for each interaction
UCLASS()
class PROJECTESTIVAL_API UMultiUserButton : public UButton
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnClickedByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnPressedByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnReleasedByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnHoveredByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnUnhoveredByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnFocusedByPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Button|Multi User")
	FMultiUserButtonEvent OnFocusLostByPlayer;
	
	// Returns every local player whose Slate user currently focuses this button
	UFUNCTION(BlueprintPure, Category = "Button|Multi User")
	TArray<ULocalPlayer*> GetFocusedPlayers() const;
	
	// Returns every local player whose pointer is currently hovering this button
	UFUNCTION(BlueprintPure, Category = "Button|Multi User")
	TArray<ULocalPlayer*> GetHoveredPlayers() const;
	
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
private:
	friend class SMultiUserButton;
	TSharedPtr<SMultiUserButton> MultiUserSlateButton;
	TSet<uint32> FocusedUserIndices;
	TSet<uint32> HoveredUserIndices;
	
	APlayerController* GetPlayerControllerForUser(uint32 UserIndex) const;
	TArray<ULocalPlayer*> GetLocalPlayersForUsers(const TSet<uint32>& UserIndices) const;
	uint32 GetCurrentEventUserIndex() const;
	
	FReply HandleSlateClicked();
	void HandleSlatePressed();
	void HandleSlateReleased();
	void HandleSlateHovered();
	void HandleSlateUnhovered();
	void HandleSlateFocusReceived();
	void HandleSlateFocusLost();
};
