#pragma once

#include "CoreMinimal.h"
#include "PlayerProfileData.generated.h"

USTRUCT(BlueprintType)
struct FProfileSummary
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ProfileId;

	UPROPERTY()
	FString ProfileName;
};

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

USTRUCT(BlueprintType)
struct FClassProgressionData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FName ClassId = NAME_None;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int64 Experience = 0;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	TArray<FProfileUpgradeData> Upgrades;
};

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