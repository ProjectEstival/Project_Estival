// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioManager.h"
#include "GameFramework/Actor.h"
#include "AudioTestbedActor.generated.h"

UCLASS()
class PROJECTESTIVAL_API AAudioTestbedActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAudioTestbedActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	TObjectPtr<UAudioManager> AudioManager;
	
	UPROPERTY(EditAnywhere, Category = "Testbed")
	bool bEnableTickTest = false;
	
private:
	UPROPERTY(EditAnywhere, Category = "Testbed")
	UFMODEvent* TestOneShotEvent;
	
	UPROPERTY(EditAnywhere, Category = "Audio Test")
	TObjectPtr<UFMODEvent> Test3DEvent;
};
