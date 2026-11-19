#pragma once

#include "Blueprint/UserWidget.h"
#include "UI/UIBackHandler.h"
#include "CoreMinimal.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "UI/CharacterSelect/CharacterSelectCard.h"
#include "UI/UIInputHandler.h"

#include "CharacterSelectScreen.generated.h"

class APlayerController;
class UPanelWidget;
class ULocalPlayer;
class ULoadout;

USTRUCT(BlueprintType)
struct FCharacterSelectPlayerChoice
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;
	UPROPERTY(BlueprintReadOnly) FText CharacterTitle;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<ULoadout> Loadout = nullptr;
	UPROPERTY(BlueprintReadOnly) TSubclassOf<AEstivalBaseCharacter> CharacterClass;
};

USTRUCT(BlueprintType)
struct FCharacterSelectCharacter
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<ULoadout> Loadout = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AEstivalBaseCharacter> CharacterClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterSelectCompleted, const TArray<FCharacterSelectPlayerChoice>&, PlayerChoices);

UCLASS()
class PROJECTESTIVAL_API UCharacterSelectScreen : public UUserWidget, public IUIBackHandler, public IUIInputHandler
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="Character Select")
	FCharacterSelectCompleted OnAllPlayersSelected;
	UFUNCTION(BlueprintCallable, Category="Character Select")
	void ConfirmCharacter(int32 CharacterIndex, APlayerController* PlayerController);
	UFUNCTION(BlueprintCallable, Category="Character Select")
	bool HandlePlayerSubmit(ULocalPlayer* LocalPlayer);
	UFUNCTION(BlueprintCallable, Category="Character Select")
	void NavigateCharacter(APlayerController* PlayerController, int32 Direction);
	void NavigateAbility(APlayerController* PlayerController, int32 Direction);
	UFUNCTION(BlueprintCallable, Category="Character Select")
	bool HandlePlayerBack(APlayerController* PlayerController);
	virtual bool HandleBack_Implementation(APlayerController* Instigator) override;
	virtual void OnBackPopped_Implementation(APlayerController* Instigator) override;
	virtual bool HandleUIInputAction_Implementation(EUIInputAction Action, APlayerController* Instigator) override;
	
protected:
	virtual void NativeOnInitialized() override;
	UPROPERTY(EditDefaultsOnly, Category="Character Select")
	TSubclassOf<UCharacterSelectCard> CharacterSelectCardClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character Select", meta=(AllowPrivateAccess="true"))
	TArray<FCharacterSelectCharacter> Characters;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character Select", meta=(AllowPrivateAccess="true"))
	float CharacterCardPadding = 20.0f;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> CharacterCardsPanel;
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCharacterSelectCard>> CharacterSelectCards;
	UPROPERTY(Transient)
	TArray<int32> AvailableCharacters;
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, int32> FocusedCharacterByPlayer;
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, int32> ConfirmedCharacterByPlayer;
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, int32> PlayerCardSlotByPlayer;
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, int32> FocusedAbilityByPlayer;
	UPROPERTY(Transient)
	bool bHasCompletedSelection = false;
	void RefreshCardStates();
	void RefreshPlayerCard(ULocalPlayer* Player);
	void CompleteIfReady();
	ULocalPlayer* ResolvePlayer(const APlayerController* PlayerController) const;
};
