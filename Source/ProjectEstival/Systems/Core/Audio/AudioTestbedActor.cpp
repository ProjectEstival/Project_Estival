// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Core/Audio/AudioTestbedActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAudioTestbedActor::AAudioTestbedActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

// Called when the game starts or when spawned
void AAudioTestbedActor::BeginPlay()
{
	Super::BeginPlay();
	
	AudioManager = GetGameInstance()->GetSubsystem<UAudioManager>();
	
	SetActorTickEnabled(bEnableTickTest);
}

// Called every frame
void AAudioTestbedActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	if (PC->WasInputKeyJustPressed(EKeys::One))
	{
		if (TestOneShotEvent) AudioManager->PlayOneShot(TestOneShotEvent);
	}
	if (PC->WasInputKeyJustPressed(EKeys::Two))
	{
		AudioManager->PlayOneShotAtLocation(Test3DEvent, GetActorLocation());
	}
}

