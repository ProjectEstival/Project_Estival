// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Core/Camera/EstivalSharedCamera.h"

// Sets default values
AEstivalSharedCamera::AEstivalSharedCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEstivalSharedCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEstivalSharedCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

