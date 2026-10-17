// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "PrototypeEnemyChar.generated.h"

UENUM(BlueprintType)
enum class EPatrolType : uint8
{
	NextInLine UMETA(DisplayName = "Next In Line"),
	RNG UMETA(DisplayName = "RNG"),
	Wander UMETA(DisplayName = "Wander")
};

UCLASS()
class PROJECTESTIVAL_API APrototypeEnemyChar : public AEstivalBaseCharacter
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "AI Patrol")
	TArray<AActor*> PatrolPoints;
	
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "AI Patrol")
	EPatrolType PatrolType = EPatrolType::NextInLine;
	
};
