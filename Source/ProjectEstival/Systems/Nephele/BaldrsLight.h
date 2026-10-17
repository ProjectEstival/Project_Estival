// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Components/SpotLightComponent.h"
#include "BaldrsLight.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBaldrsLightPlayerEvent, ACharacter*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBaldrsLightPlayerDamagedEvent, ACharacter*, Player, float, Damage);

USTRUCT()
struct FBaldrsLightPlayerState
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<ACharacter> Player = nullptr;
	float DamagePerTick = 0.0f;
	bool bInsideLight = true;
};

UCLASS()
class ABaldrsLight : public AActor
{
	GENERATED_BODY()
	
public:
	ABaldrsLight();
	
	// Player definition
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Baldr's Light|Players")
	TSubclassOf<ACharacter> PlayerBaseClass;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Baldr's Light|Players")
	TArray<TObjectPtr<ACharacter>> Players;
	
	// Light settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light", meta = (ClampMin = "0.0"))
	float LightRadius = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light", meta = (ClampMin = "0.0"))
	float MovementSpeed = 5.0f;
	
	// Corruption damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Damage", meta = (ClampMin = "0.0"))
	float CorruptionRate = 1.0f; // per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Damage", meta = (ClampMin = "0.0"))
	float CorruptionRecoveryRate = 2.0f; // per second
	// The default settings give:
	// 0-2c: 1dmg/s  ->  2-4c: 2dmg/s  ->  4-6c: 4dmg/s  ->  6-8c: 8dmg/s  ->  8-10c: 16dmg/s
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Damage", meta = (ClampMin = "0.0"))
	float CorruptionPerTier = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Damage", meta = (ClampMin = "0.0"))
	float TierExponent = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Damage", meta = (ClampMin = "0.0"))
	float DamageInterval = 1.0f;
		
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerEnteredLight;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerExitedLight;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerCured;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerDamagedEvent OnPlayerDamaged;
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
	// Events
	UFUNCTION(BlueprintNativeEvent, Category = "Baldr's Light|Events")
	void PlayerEnteredLight(ACharacter* Player);
	virtual void PlayerEnteredLight_Implementation(ACharacter* Player);
	UFUNCTION(BlueprintNativeEvent, Category = "Baldr's Light|Events")
	void PlayerExitedLight(ACharacter* Player);
	virtual void PlayerExitedLight_Implementation(ACharacter* Player);
	UFUNCTION(BlueprintNativeEvent, Category = "Baldr's Light|Events")
	void PlayerCured(ACharacter* Player);
	virtual void PlayerCured_Implementation(ACharacter* Player);
	UFUNCTION(BlueprintNativeEvent, Category = "Baldr's Light|Events")
	void PlayerDamaged(ACharacter* Player, float Damage);
	virtual void PlayerDamaged_Implementation(ACharacter* Player, float Damage);
	
	// Light projection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Baldr's Light|Visuals")
	TObjectPtr<USpotLightComponent> SafeAreaSpotlight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "2.0"))
	float SpotlightHeight = 650.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float SpotlightEdgeSoftness = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "1.0"))
	float SpotlightAttenuationMult = 2.0f;
	
	// Debug gizmo toggles 
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	void DrawEditorDebug() const;
#endif
	UPROPERTY(EditAnywhere, Category = "Baldr's Light|Debug")
	bool bShowLightDebug = true;
	UPROPERTY(EditAnywhere, Category = "Baldr's Light|Debug")
	bool bShowSpotlightCone = false;
	UPROPERTY(EditAnywhere, Category = "Baldr's Light|Debug")
	bool bShowEffectRadius = true;
	
private:
	UPROPERTY()
	TArray<FBaldrsLightPlayerState> PlayerStates;
	void FindPlayers();
	FVector CalculatePlayersBarycenter() const;
	void UpdatePlayerStates(float DeltaTime);
	
	void UpdateSpotlight() const;
	void UpdateLightPosition(float DeltaTime);
	
	FGameplayTag CorruptionTag;
	FGameplayTag HealthTag;
	void UpdateCorruption(FBaldrsLightPlayerState& State, float DeltaTime);
	float GetDamagePerTick(float Corruption) const;
};
