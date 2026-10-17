// Fill out your copyright notice in the Description page of Project Settings.

#include "BaldrsLight.h"

#include "Systems/Mike/Combat/ResourceComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABaldrsLight::ABaldrsLight()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	SafeAreaSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SafeAreaSpotlight"));
	SafeAreaSpotlight->SetupAttachment(SceneRoot);
	SafeAreaSpotlight->SetRelativeLocation(FVector(0.0f, 0.0f, SpotlightHeight));
	SafeAreaSpotlight->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	SafeAreaSpotlight->SetMobility(EComponentMobility::Movable);
	SafeAreaSpotlight->SetVisibility(true);
	SafeAreaSpotlight->SetHiddenInGame(false);
	SafeAreaSpotlight->SetCastShadows(false);
	SafeAreaSpotlight->SetIntensityUnits(ELightUnits::Lumens);
	SafeAreaSpotlight->SetIntensity(100.0f);
	SafeAreaSpotlight->SetLightColor(FLinearColor::MakeFromHSV8(34, 179, 255));
}
void ABaldrsLight::BeginPlay()
{
	Super::BeginPlay();
	
	FindPlayers();
	CorruptionTag = FGameplayTag::RequestGameplayTag(FName("Stats.Resources.Corruption"));
	HealthTag = FGameplayTag::RequestGameplayTag(FName("Stats.Resources.Health"));
	
	PlayerStates.Reset();
	
	for (ACharacter* Player : Players)
	{
		if (!IsValid(Player)) continue;
		FBaldrsLightPlayerState State;
		State.Player = Player;
		State.bInsideLight = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) <= FMath::Square(LightRadius);
		PlayerStates.Add(State);
	}
	
	UpdateSpotlight();
}
void ABaldrsLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateLightPosition(DeltaTime);
	UpdatePlayerStates(DeltaTime);
	UpdateSpotlight();
	
#if WITH_EDITOR
	DrawEditorDebug();
#endif
}

void ABaldrsLight::FindPlayers()
{
	Players.Reset();
	if (!PlayerBaseClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Baldr's Light: PlayerBaseClass is not set."));
		return;
	}
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), PlayerBaseClass, FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (ACharacter* Character = Cast<ACharacter>(Actor)) Players.Add(Character);
	}
	UE_LOG(LogTemp, Log, TEXT("Baldr's Light found %d players."), Players.Num());
}

// Movement
FVector ABaldrsLight::CalculatePlayersBarycenter() const
{
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;
	for (ACharacter* Player : Players)
	{
		if (!IsValid(Player)) continue;
		Sum += Player->GetActorLocation();
		++Count;
	}
	if (Count == 0) return GetActorLocation();
	return Sum / static_cast<float>(Count);
}
void ABaldrsLight::UpdateLightPosition(float DeltaTime)
{
	if (Players.IsEmpty()) return;
	const FVector Target = CalculatePlayersBarycenter();
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), Target, DeltaTime, MovementSpeed);
	SetActorLocation(NewLocation);
}
void ABaldrsLight::UpdateSpotlight() const
{
	// Ensure the light's projection on the ground is indicative of the safe zone
	
	if (!SafeAreaSpotlight) return;
	const float Radius = FMath::Max(0.0f, LightRadius);
	const float Height = FMath::Max(1.0f, SpotlightHeight);
	
	const float HalfAngle = FMath::RadiansToDegrees(FMath::Atan(Radius / Height));
	const float OuterAngle = FMath::Clamp(HalfAngle, 1.0f, 80.0f);
	const float InnerAngle = FMath::Max(0.0f, OuterAngle - SpotlightEdgeSoftness);
	const float DistanceToEdge = FMath::Sqrt(FMath::Square(Height) + FMath::Square(Radius));
	const float AttenuationRadius = DistanceToEdge * SpotlightAttenuationMult;
	
	SafeAreaSpotlight->SetRelativeLocation(FVector(0.0f, 0.0f, Height));
	SafeAreaSpotlight->SetOuterConeAngle(OuterAngle);
	SafeAreaSpotlight->SetInnerConeAngle(InnerAngle);
	SafeAreaSpotlight->SetAttenuationRadius(AttenuationRadius);
}

// Corruption
void ABaldrsLight::UpdatePlayerStates(float DeltaTime)
{
	for (FBaldrsLightPlayerState& State : PlayerStates)
	{
		ACharacter* Player = State.Player;
		if (!IsValid(Player)) continue;
		const float DistanceSquared = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation());
		const bool bIsInside = DistanceSquared <= FMath::Square(LightRadius);
		
		// Events
		if (bIsInside != State.bInsideLight)
		{
			State.bInsideLight = bIsInside;
			if (bIsInside)
			{
				PlayerEnteredLight(Player);
				OnPlayerEnteredLight.Broadcast(Player);
			}
			else
			{
				PlayerExitedLight(Player);
				OnPlayerExitedLight.Broadcast(Player);
			}
		}
		
		UpdateCorruption(State, DeltaTime);
	}
}
void ABaldrsLight::UpdateCorruption(FBaldrsLightPlayerState& State, float DeltaTime)
{
	ACharacter* Player = State.Player;
	if (!IsValid(Player) || DeltaTime <= 0.0f) return;
	
	// Add/remove corruption
	UResourceComponent* ResourceManager = Player->FindComponentByClass<UResourceComponent>();
	if (!ResourceManager) return;
	ResourceManager->Modify(CorruptionTag, State.bInsideLight ? -CorruptionRecoveryRate * DeltaTime : CorruptionRate * DeltaTime);
	
	// Debug player health / corruption [DELETE ME]
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Health: %.2f"), ResourceManager->GetFloat(HealthTag)));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Corruption: %.2f"), ResourceManager->GetFloat(CorruptionTag)));
	
	const float Corruption = ResourceManager->GetFloat(CorruptionTag);
	// Stop damage
	if (Corruption <= 0.0f)
	{
		// Cured event
		if (State.DamagePerTick > 0.0f)
		{
			PlayerCured(Player);
			OnPlayerCured.Broadcast(Player);
		}
		State.DamagePerTick = 0.0f;
		return;
	}
	
	// Apply damage
	State.DamagePerTick += DeltaTime;
	while (State.DamagePerTick >= DamageInterval)
	{
		State.DamagePerTick -= DamageInterval;
		const float Damage = GetDamagePerTick(Corruption);
		ResourceManager->Modify(HealthTag, -Damage);
		
		PlayerDamaged(Player, Damage);
		OnPlayerDamaged.Broadcast(Player, Damage);
	}
}
float ABaldrsLight::GetDamagePerTick(float Corruption) const
{
	const int32 Tier = FMath::FloorToInt(Corruption / CorruptionPerTier);
	return FMath::Pow(TierExponent, static_cast<float>(Tier));
}

// Event implementations
void ABaldrsLight::PlayerEnteredLight_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerExitedLight_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerCured_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerDamaged_Implementation(ACharacter* Player, float Damage) { }

#if WITH_EDITOR
void ABaldrsLight::DrawEditorDebug() const
{
    UWorld* World = GetWorld();
    if (!World || World->IsGameWorld() || !bShowLightDebug) return;
    const FVector FloorCenter = GetActorLocation();
	
    // Effect area circle on the XY floor plane.
    if (bShowEffectRadius) DrawDebugCircle(World, FloorCenter, LightRadius, 64, FColor::Green, false, 0.0f, 0, 2.5f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	
    // Mark the physical spotlight's position.
    if (!SafeAreaSpotlight) return;
    const FVector LightLocation = SafeAreaSpotlight->GetComponentLocation();
    DrawDebugSphere(World, LightLocation, 25.0f, 12, FColor::Yellow, false, 0.0f, 0, 2.0f);
	
    // Outer cone.
    DrawDebugLine(World, LightLocation, FloorCenter, FColor::Yellow, false, 0.0f, 0, 1.5f);
	
    // Inner cone.
    if (!bShowSpotlightCone) return;
    const float HalfAngleRadians = FMath::DegreesToRadians(SafeAreaSpotlight->OuterConeAngle);
    DrawDebugCone(World, LightLocation, SafeAreaSpotlight->GetForwardVector(), SafeAreaSpotlight->AttenuationRadius, HalfAngleRadians, HalfAngleRadians, 32, FColor::Yellow, false, 0.0f, 0, 1.5f);
}
#endif