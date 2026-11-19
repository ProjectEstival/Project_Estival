#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/CharacterSelect/CharacterSelectScreen.h"
#include "DungeonGameMode.generated.h"

class UUserWidget;
class ACompanionAIController;

UCLASS()
class PROJECTESTIVAL_API ADungeonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleAllPlayersSelected(const TArray<FCharacterSelectPlayerChoice>& PlayerChoices);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CharacterSelectBaseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InGameBaseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Characters")
	TSubclassOf<ACompanionAIController> CompanionAIControllerClass;

private:
	void InitializeBaseWidget() const;
	bool bHasHandledPlayerSelections = false;
};
