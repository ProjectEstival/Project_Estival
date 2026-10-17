#include "BaldrsLight.h"

#include "Systems/Mike/Combat/ResourceComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABaldrsLight::ABaldrsLight()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	// Lantern
	Lantern = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lantern"));
	Lantern->SetupAttachment(SceneRoot);
	Lantern->SetMobility(EComponentMobility::Movable);
	Lantern->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Lantern->SetGenerateOverlapEvents(false);
	Lantern->SetCastShadow(false);
	
	// Default engine sphere. Artists can replace this mesh in the Blueprint.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (SphereMesh.Succeeded())
	{
		Lantern->SetStaticMesh(SphereMesh.Object);
		Lantern->SetRelativeScale3D(FVector(0.35f));
		Lantern->SetRelativeLocation(FVector(0.0f, 0.0f, LanternHeight));
	}
	
	// Spotlight
	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SafeAreaSpotlight"));
	Spotlight->SetupAttachment(Lantern);
	Spotlight->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	Spotlight->SetMobility(EComponentMobility::Movable);
	Spotlight->SetVisibility(true);
	Spotlight->SetHiddenInGame(false);
	Spotlight->SetCastShadows(false);
	Spotlight->SetIntensityUnits(ELightUnits::Lumens);
	Spotlight->SetIntensity(100.0f);
	Spotlight->SetLightColor(FLinearColor::MakeFromHSV8(34, 179, 255));
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
	
	AnimationTime = 0.0f;
	HorizontalVelocity = FVector::ZeroVector;
	bMovementInitialized = true;
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
TArray<TObjectPtr<ACharacter>> ABaldrsLight::GetPlayersInLight()
{
	TArray<TObjectPtr<ACharacter>> PlayersInLight;
	for (FBaldrsLightPlayerState& State : PlayerStates)
	{
		ACharacter* Player = State.Player;
		if (!IsValid(Player)) continue;
		if (State.bInsideLight) PlayersInLight.Add(Player);
	}
	return PlayersInLight;
}
TArray<TObjectPtr<ACharacter>> ABaldrsLight::GetPlayersOutsideLight()
{
	TArray<TObjectPtr<ACharacter>> PlayersInLight;
	for (FBaldrsLightPlayerState& State : PlayerStates)
	{
		ACharacter* Player = State.Player;
		if (!IsValid(Player)) continue;
		if (!State.bInsideLight) PlayersInLight.Add(Player);
	}
	return PlayersInLight;
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
	if (DeltaTime <= 0.0f || Players.IsEmpty()) return;
	AnimationTime += DeltaTime;
	
	// HORIZONTAL POSITION
	
	const FVector CurrentPosition = GetActorLocation();
	const FVector Target = CalculatePlayersBarycenter();
	
	// Follow the players horizontally only
	const FVector Position = FVector(CurrentPosition.X, CurrentPosition.Y, 0.0f);
	const FVector TargetPosition = FVector(Target.X, Target.Y, 0.0f);
	const FVector Displacement = TargetPosition - Position;
	
	// Damped spring: acceleration depends on both distance and velocity
	const FVector Acceleration = Displacement * SpringStiffness - HorizontalVelocity * SpringDamping;
	HorizontalVelocity += Acceleration * DeltaTime;
	FVector NewPosition = Position + HorizontalVelocity * DeltaTime;
	SetActorLocation(NewPosition);
	
	// VERTICAL POSITION
	
	// Normalize speed
    const float Speed = HorizontalVelocity.Size();
    const float Intensity = FMath::Clamp(Speed / DipSpeed, 0.0f, 1.0f);
	
	// Apply vertical position
    CurrentDip = FMath::FInterpTo(CurrentDip, Intensity * DipWhileMoving, DeltaTime, CosmeticMovementResponseSpeed); // Dip as the lantern moves faster
	const float BobOffset = BobAmplitude * FMath::Sin(AnimationTime * 2.0f * PI * BobFrequency); // Bob up/down
    Lantern->SetRelativeLocation(FVector(0.0f, 0.0f, LanternHeight + BobOffset - CurrentDip));
	
	// ROTATION
	
	// Tilt toward the direction of travel
	const FVector LocalVelocity = GetActorTransform().InverseTransformVectorNoScale(HorizontalVelocity);
	FRotator TargetRotation = FRotator::ZeroRotator;
	if (Speed > 1.0f)
	{
		TargetRotation.Pitch = -FMath::Clamp(LocalVelocity.X / TiltSpeed, -1.0f, 1.0f) * TiltWhileMoving;
		TargetRotation.Roll = FMath::Clamp(LocalVelocity.Y / TiltSpeed, -1.0f, 1.0f) * TiltWhileMoving;
	}
	CurrentLanternRotation = FMath::RInterpTo(CurrentLanternRotation, TargetRotation, DeltaTime, CosmeticMovementResponseSpeed);
	Lantern->SetRelativeRotation(CurrentLanternRotation);
}
void ABaldrsLight::UpdateSpotlight() const
{
	// Ensure the light's projection on the ground is indicative of the safe zone
	
	if (!Spotlight) return;
	const float Radius = FMath::Max(0.0f, LightRadius);
	const float Height = FMath::Max(1.0f, Lantern->GetRelativeLocation().Z);
	
	const float HalfAngle = FMath::RadiansToDegrees(FMath::Atan(Radius / Height));
	const float OuterAngle = FMath::Clamp(HalfAngle, 1.0f, 80.0f);
	const float InnerAngle = FMath::Max(0.0f, OuterAngle - SpotlightEdgeSoftness);
	const float DistanceToEdge = FMath::Sqrt(FMath::Square(Height) + FMath::Square(Radius));
	const float AttenuationRadius = DistanceToEdge * SpotlightAttenuationMult;
	
	Spotlight->SetRelativeLocation(FVector(0.0f, 0.0f, Height));
	Spotlight->SetOuterConeAngle(OuterAngle);
	Spotlight->SetInnerConeAngle(InnerAngle);
	Spotlight->SetAttenuationRadius(AttenuationRadius);
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
	
	// Determine the current corruption tier
	const float Corruption = ResourceManager->GetFloat(CorruptionTag);
	const int CurrentTier = Corruption > 0.0f ? FMath::CeilToInt(Corruption / CorruptionPerTier) : 0;
	if (State.CorruptionTier != CurrentTier) BroadcastCorruptionTierChanges(State, CurrentTier);
	
	// Skip damage if no corruption
	if (CurrentTier == 0)
	{
		State.DamageIntervalTimer = 0.0f;
		return;
	}
	
	// Apply damage at regular intervals
	State.DamageIntervalTimer += DeltaTime;
	while (State.DamageIntervalTimer >= DamageInterval)
	{
		State.DamageIntervalTimer -= DamageInterval;
		
		const float Damage = GetDamagePerTick(CurrentTier);
		ResourceManager->Modify(HealthTag, -Damage);
		
		PlayerDamaged(Player, Damage);
		OnPlayerDamaged.Broadcast(Player, Damage);
	}
}
float ABaldrsLight::GetDamagePerTick(const int Tier) const
{
	return FMath::Pow(TierExponent, Tier);
}
void ABaldrsLight::BroadcastCorruptionTierChanges(FBaldrsLightPlayerState& State, int Tier)
{
	State.CorruptionTier = Tier;
	PlayerCorruptionTierChanged(State.Player, Tier);
	OnPlayerCorruptionTierChanged.Broadcast(State.Player, Tier);
	
	if (Tier == 0)
	{
		PlayerCured(State.Player);
		OnPlayerCured.Broadcast(State.Player);
	}
}

// Event implementations
void ABaldrsLight::PlayerEnteredLight_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerExitedLight_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerCured_Implementation(ACharacter* Player) { }
void ABaldrsLight::PlayerDamaged_Implementation(ACharacter* Player, float Damage) { }
void ABaldrsLight::PlayerCorruptionTierChanged_Implementation(ACharacter* Player, int Tier) { }

#if WITH_EDITOR
void ABaldrsLight::DrawEditorDebug() const
{
    UWorld* World = GetWorld();
    if (!World || !bShowLightDebug) return;
    const FVector FloorCenter = GetActorLocation();
	
    // Effect area circle on the XY floor plane.
    if (bShowEffectRadius) DrawDebugCircle(World, FloorCenter, LightRadius, 64, FColor::Green, false, 0.0f, 0, 2.5f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	
    // Mark the physical spotlight's position.
    if (!Spotlight) return;
    const FVector LightLocation = Spotlight->GetComponentLocation();
    DrawDebugSphere(World, LightLocation, 25.0f, 12, FColor::Yellow, false, 0.0f, 0, 2.0f);
	
    // Outer cone.
    DrawDebugLine(World, LightLocation, FloorCenter, FColor::Yellow, false, 0.0f, 0, 1.5f);
	
    // Inner cone.
    if (!bShowSpotlightCone) return;
    const float HalfAngleRadians = FMath::DegreesToRadians(Spotlight->OuterConeAngle);
    DrawDebugCone(World, LightLocation, Spotlight->GetForwardVector(), Spotlight->AttenuationRadius, HalfAngleRadians, HalfAngleRadians, 32, FColor::Yellow, false, 0.0f, 0, 1.5f);
}
#endif