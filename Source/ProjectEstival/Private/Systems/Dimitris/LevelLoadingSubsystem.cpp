
#include "Systems/Dimitris/LevelLoadingSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

void ULevelLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this,&ULevelLoadingSubsystem::HandlePostLoadMap);
}

void ULevelLoadingSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

void ULevelLoadingSubsystem::LoadLevel(FName LevelName)
{
	if (bIsLoading || LevelName.IsNone())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (!GameInstance)
	{
		return;
	}

	UWorld* World = GameInstance->GetWorld();

	if (!World)
	{
		return;
	}

	bIsLoading = true;

	OnLoadingStarted.Broadcast();
	OnLoadingProgress.Broadcast(0.0f);

	UGameplayStatics::OpenLevel(World,LevelName);
}

void ULevelLoadingSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!bIsLoading || !LoadedWorld)
	{
		return;
	}

	bIsLoading = false;

	OnLoadingProgress.Broadcast(1.0f);
	OnLoadingFinished.Broadcast();
}
