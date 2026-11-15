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
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Profile")
	int32 SchemaVersion = 1;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Profile")
	FPlayerProfileData Profile;
};
