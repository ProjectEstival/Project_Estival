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
	
	/** The modifiers picked for the current floor. */
	UFUNCTION(BlueprintPure, Category = "Run")
	TArray<FFloorModifier> GetFloorModifiers() const { return ActiveModifiers; }

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<APlayerBase>> Party;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LevelManager;

	UPROPERTY(Transient)
	TObjectPtr<AEstivalBaseCharacter> CurrentBoss;

	UPROPERTY(Transient)
	TArray<FFloorModifier> ActiveModifiers;

	/** Space between heroes when the party is lined up at a floor's start. */
	static constexpr float PartySpacing = 150.f;
	int32 FloorIndex = 0;
	EDungeonPhase Phase = EDungeonPhase::LoadingFloor;
	FDelegateHandle SpawnHandle;
	/** Characters that spawned or came out of the pool this frame. A set, so one that did both is only in here once. */
	TSet<TWeakObjectPtr<AEstivalBaseCharacter>> PendingCharacters;

	void SpawnParty();
	void RequestFloor();
	void RollFloorModifiers();
	void ApplyFloorModifiers(AEstivalBaseCharacter* Character);
	void OnActorAppeared(AActor* Actor);
	void ApplyPendingFloorModifiers();
	void AwardFragments(int32 Amount) const;
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
