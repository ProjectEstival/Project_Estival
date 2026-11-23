// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EnemyBaseCharacter.generated.h"

class UStateTree;
class UAnimMontage;

UCLASS()
class PROJECTESTIVAL_API AEnemyBaseCharacter : public AEstivalBaseCharacter
{
	GENERATED_BODY()
	
public: 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tree")
	TObjectPtr<UStateTree> StateTreeAsset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Animaions")
	UAnimMontage* HitMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Animaions")
	UAnimMontage* DeathMontage;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void Dead();
	
	UFUNCTION()
	void DeathAnim();

private:
	
	UFUNCTION()
	void HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	
};