#pragma once

#include "CoreMinimal.h"
#include "GameModeHelpers.h"
#include "GameFramework/GameModeBase.h"
#include "DungeonGameMode.generated.h"

class AEstivalBaseCharacter;
class APlayerBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorEvent, int32, Floor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunEnded, ERunResult, Result);

UCLASS()
class PROJECTESTIVAL_API ADungeonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADungeonGameMode();

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnFloorEvent OnFloorStarted;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnFloorEvent OnFloorCleared;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRunEnded OnRunEnded;
	
	virtual void StartPlay() override;

	/** The level manager calls this when the floor is built. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void FloorReady(const FTransform& PartyStart, AEstivalBaseCharacter* Boss);

	UFUNCTION(Exec)
	void BossDefeated();

	UFUNCTION(BlueprintCallable, Exec, Category = "Run")
	void ContinueRun();

	UFUNCTION(BlueprintCallable, Exec, Category = "Run")
	void LeaveRun();

	/** The current floor, starting at 1. */
	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetFloor() const { return FloorIndex + 1; }

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetFloorCount() const { return Floors.Num(); }

	UFUNCTION(BlueprintPure, Category = "Run")
	EDungeonPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category = "Run")
	TArray<APlayerBase*> GetParty() const;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TArray<FDungeonFloor> Floors;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "1", ClampMax = "4"))
	int32 PartySize = 4;
	
	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0", ClampMax = "1"))
	float PartyReviveHealthPercentage = 1.f;

	/** Opened when the run ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSoftObjectPtr<UWorld> HubMap;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0.1", Units = "s"))
	float ReturnToHubDelay = 3.f;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Run")
	TSubclassOf<APlayerBase> ChoosePlayerHero(APlayerController* Player);

	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<APlayerBase>> Party;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LevelManager;

	UPROPERTY(Transient)
	TObjectPtr<AEstivalBaseCharacter> CurrentBoss;

	/** Space between heroes when the party is lined up at a floor's start. */
	static constexpr float PartySpacing = 150.f;
	int32 FloorIndex = 0;
	EDungeonPhase Phase = EDungeonPhase::LoadingFloor;

	void SpawnParty();
	void RequestFloor();
	void EndRun(ERunResult Result);
	void ReturnToHub() const;
	TArray<TSubclassOf<APlayerBase>> GetHeroes() const;
	TSubclassOf<APlayerBase> ChooseBotHero() const;
	FTransform GetPlayerStartTransform();

	UFUNCTION()
	void OnPartyMemberDied();

	UFUNCTION()
	void OnBossDied();
};
