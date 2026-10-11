// Fill out your copyright notice in the Description page of Project Settings.


#include "DevConsoleTestActor.h"

// Sets default values
ADevConsoleTestActor::ADevConsoleTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADevConsoleTestActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADevConsoleTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADevConsoleTestActor::TestLogs()
{
	UE_LOG(LogTemp, Display, TEXT("DevConsole: basic log test"));
	UE_LOG(LogTemp, Warning, TEXT("DevConsole: warning test"));
	UE_LOG(LogTemp, Error, TEXT("DevConsole: error test"));
	
	Async(EAsyncExecution::ThreadPool, []()
	{
		UE_LOG(LogTemp, Warning, TEXT("DevConsole: worker-thread test"));
	});
}

