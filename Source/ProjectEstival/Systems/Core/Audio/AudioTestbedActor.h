
#pragma once

#include "CoreMinimal.h"
#include "AudioManager.h"
#include "GameFramework/Actor.h"
#include "FMODAudioComponent.h"
#include "AudioTestbedActor.generated.h"

class UFMODSnapshot;

UCLASS()
class PROJECTESTIVAL_API AAudioTestbedActor : public AActor
{
	GENERATED_BODY()

public:
	AAudioTestbedActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void PrintTest(const FString& Message) const;

	UPROPERTY(Transient)
	TObjectPtr<UAudioManager> AudioManager;

	UPROPERTY(EditAnywhere, Category="Testbed")
	bool bEnableTickTest = true;

	UPROPERTY(EditAnywhere, Category="Audio Test|Events")
	TObjectPtr<UFMODEvent> TestOneShotEvent;

	UPROPERTY(EditAnywhere, Category="Audio Test|Events")
	TObjectPtr<UFMODEvent> Test3DEvent;

	UPROPERTY(EditAnywhere, Category="Audio Test|Events")
	TObjectPtr<UFMODEvent> LoopEvent;

	UPROPERTY(EditAnywhere, Category="Audio Test|Events")
	TObjectPtr<UFMODEvent> AmbienceEvent;

	UPROPERTY(EditAnywhere, Category="Audio Test|Music")
	TObjectPtr<UFMODEvent> TestMusicA;

	UPROPERTY(EditAnywhere, Category="Audio Test|Music")
	TObjectPtr<UFMODEvent> TestMusicB;

	UPROPERTY(EditAnywhere, Category="Audio Test|Parameters")
	FName MusicParameterName = TEXT("Intensity");

	UPROPERTY(EditAnywhere, Category="Audio Test|Parameters")
	FName LoopParameterName = TEXT("Intensity");

	UPROPERTY(EditAnywhere, Category="Audio Test|Parameters")
	FName GlobalParameterName = TEXT("GlobalIntensity");

	UPROPERTY(Transient)
	TObjectPtr<UFMODAudioComponent> AmbienceComponent;

	UPROPERTY(Transient)
	TObjectPtr<UFMODAudioComponent> TestAudioComponent;

	UPROPERTY(Transient)
	TObjectPtr<UFMODAudioComponent> SecondaryAudioComponent;
	
	UPROPERTY(EditAnywhere, Category="Audio Test|Snapshots")
	TObjectPtr<UFMODSnapshot> TestSnapshot;
};
