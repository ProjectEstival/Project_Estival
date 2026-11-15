#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "ProfileDisplayEntry.generated.h"

class ULocalPlayer;
class UTextBlock;
class UBorder;

// Entry for an assigned local player's name and color for the main-menu player list
UCLASS()
class PROJECTESTIVAL_API UProfileDisplayEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Main Menu|Ready Players")
	bool SetPlayer(ULocalPlayer* LocalPlayer);
	
	UFUNCTION(BlueprintPure, Category = "Main Menu|Ready Players")
	ULocalPlayer* GetPlayer() const { return Player; }
	
	UFUNCTION(BlueprintPure, Category = "Main Menu|Ready Players")
	int32 GetColorSlot() const { return ColorSlot; }
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ProfileNameText;
	
private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Main Menu|Ready Players", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULocalPlayer> Player;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Main Menu|Ready Players", meta = (AllowPrivateAccess = "true"))
	int32 ColorSlot = INDEX_NONE;
};
