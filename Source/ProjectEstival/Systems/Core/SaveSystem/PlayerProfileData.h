#pragma once

#include "CoreMinimal.h"
#include "PlayerProfileData.generated.h"

// Summary for profile selection cards
USTRUCT(BlueprintType)
struct FProfileSummary
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ProfileId;

	UPROPERTY()
	FString ProfileName;
};

// Upgrade level
USTRUCT(BlueprintType)
struct FProfileUpgradeData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FName UpgradeId = NAME_None;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int32 UpgradeLevel = 0;
};

// Class level
USTRUCT(BlueprintType)
struct FClassProgressionData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FName ClassId = NAME_None;
		
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	TArray<FProfileUpgradeData> Upgrades;
};

// Profile level
USTRUCT(BlueprintType)
struct FPlayerProfileData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	FGuid ProfileId;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FString ProfileName;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	TArray<FClassProgressionData> ClassProgressions;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int32 UnspentFragments = 0;
};