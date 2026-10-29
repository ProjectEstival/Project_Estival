
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelLoadingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnLoadingProgress, float, Progress);

UCLASS()
class PROJECTESTIVAL_API ULevelLoadingSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Level Loading")
	FOnLoadingStarted OnLoadingStarted;

	UPROPERTY(BlueprintAssignable, Category="Level Loading")
	FOnLoadingFinished OnLoadingFinished;

	UPROPERTY(BlueprintAssignable, Category="Level Loading")
	FOnLoadingProgress OnLoadingProgress;

	UFUNCTION(BlueprintCallable, Category="Level Loading")
	void LoadLevel(FName LevelName);

	UFUNCTION(BlueprintPure, Category="Level Loading")
	bool IsLoading() const { return bIsLoading; }

	virtual void Initialize(
		FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:

	UFUNCTION()
	void HandlePostLoadMap(UWorld* LoadedWorld);

	bool bIsLoading = false;
};
