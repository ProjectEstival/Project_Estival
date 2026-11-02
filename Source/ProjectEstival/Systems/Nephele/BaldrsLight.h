#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BaldrsLight.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBaldrsLightPlayerEvent, ACharacter*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBaldrsLightPlayerDamagedEvent, ACharacter*, Player, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBaldrsLightPlayerCorruptionEvent, ACharacter*, Player, int, Tier);

USTRUCT()
struct FBaldrsLightPlayerState
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<ACharacter> Player = nullptr;
	float DamageIntervalTimer = 0.0f;
	int CorruptionTier = 0;
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
	TArray<TObjectPtr<ACharacter>> GetPlayersInLight();
	TArray<TObjectPtr<ACharacter>> GetPlayersOutsideLight();
	
	// Light settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light", meta = (ClampMin = "0.0"))
	float LightRadius = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light", meta = (ClampMin = "0.0"))
	float MovementSpeed = 5.0f;
	
	// Lantern model
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Baldr's Light|Visuals")
	TObjectPtr<UStaticMeshComponent> Lantern;
	
	// Spring movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.1"))
	float SpringStiffness = 16.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float SpringDamping = 3.0f;
	
	// Vertical bob
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float BobAmplitude = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float BobFrequency = 0.35f;
	
	// Vertical dip down while moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float DipWhileMoving = 15.0f; // Maximum downward offset in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "1.0"))
	float DipSpeed = 250.0f; // Speed at which the duck reaches its maximum
	
	// Tilting towards the movement direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float TiltWhileMoving = 12.0f; // Maximum tilt in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "1.0"))
	float TiltSpeed = 250.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Movement", meta = (ClampMin = "0.0"))
	float CosmeticMovementResponseSpeed = 4.0f;
	
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerEnteredLight;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerExitedLight;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerEvent OnPlayerCured;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerDamagedEvent OnPlayerDamaged;
	UPROPERTY(BlueprintAssignable, Category = "Baldr's Light|Events")
	FBaldrsLightPlayerCorruptionEvent OnPlayerCorruptionTierChanged;
	
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif
	
	// Corruption damage
	float GetDamagePerTick(int Tier) const;
	
protected:
	virtual void BeginPlay() override;
	
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
	UFUNCTION(BlueprintNativeEvent, Category = "Baldr's Light|Events")
	void PlayerCorruptionTierChanged(ACharacter* Player, int Tier);
	virtual void PlayerCorruptionTierChanged_Implementation(ACharacter* Player, int Tier);
	
	// Light projection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Baldr's Light|Visuals")
	TObjectPtr<USpotLightComponent> Spotlight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "2.0"))
	float LanternHeight = 450.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float SpotlightEdgeSoftness = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baldr's Light|Visuals", meta = (ClampMin = "1.0"))
	float SpotlightAttenuationMult = 2.0f;
	
	// Debug gizmo toggles 
#if WITH_EDITOR
	void DrawEditorDebug() const;
#endif
	UPROPERTY(EditAnywhere, Category = "Baldr's Light|Debug")
	bool bShowLightDebug = false;
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
	bool bMovementInitialized = false;
	float AnimationTime = 0.0f;
	float CurrentDip = 0.0f;
	FVector HorizontalVelocity = FVector::ZeroVector;
	FRotator CurrentLanternRotation = FRotator::ZeroRotator;
	
	FGameplayTag CorruptionTag;
	FGameplayTag HealthTag;
	void UpdateCorruption(FBaldrsLightPlayerState& State, float DeltaTime);
	void BroadcastCorruptionTierChanges(FBaldrsLightPlayerState& State, int Tier);
};
