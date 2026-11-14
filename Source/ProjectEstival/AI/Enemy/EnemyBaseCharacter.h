// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "EnemyBaseCharacter.generated.h"

class UStateTree;
UCLASS()
class PROJECTESTIVAL_API AEnemyBaseCharacter : public AEstivalBaseCharacter
{
	GENERATED_BODY()
	
public: 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tree")
	TObjectPtr<UStateTree> StateTreeAsset;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void Dead();
	
};
