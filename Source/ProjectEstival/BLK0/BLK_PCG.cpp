// Fill out your copyright notice in the Description page of Project Settings.


#include "BLK0/BLK_PCG.h"

// Sets default values
ABLK_PCG::ABLK_PCG()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABLK_PCG::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABLK_PCG::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

