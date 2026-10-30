#pragma once

#include "GameFramework/SaveGame.h"
#include "PlayerProfileData.h"
#include "CoreMinimal.h"
#include "PlayerProfileSaveGame.generated.h"

UCLASS()
class PROJECTESTIVAL_API UPlayerProfileSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame)
	int32 SchemaVersion = 1;

	UPROPERTY(SaveGame)
	FPlayerProfileData Profile;
};
