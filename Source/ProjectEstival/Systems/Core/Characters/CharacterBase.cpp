// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_LookAtComponent.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Systems/Core/TagDefinitions.h"
#include "Systems/Mike/Combat/Loadouts/AimComponent.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

AEstivalBaseCharacter::AEstivalBaseCharacter()
{
	// components do their own ticking, the character only ticks while an ability makes it face the aim
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	LookAt = CreateDefaultSubobject<UAC_LookAtComponent>(TEXT("LookAt"));
	StatusEffects = CreateDefaultSubobject<UAC_StatusEffectComponent>(TEXT("StatusEffects"));
	Statistics = CreateDefaultSubobject<UStatisticsComponent>(TEXT("Statistics"));
	Resources = CreateDefaultSubobject<UResourceComponent>(TEXT("Resources"));
	Loadout = CreateDefaultSubobject<ULoadoutComponent>(TEXT("Loadout"));
	Aim = CreateDefaultSubobject<UAimComponent>(TEXT("Aim"));

	// rotation is driven by the LookAt component
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	MovementBlockingTags.AddTag(TagDefinitions::Status_Stunned);
	AbilityBlockingTags.AddTag(TagDefinitions::Status_Stunned);
}

float AEstivalBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsAlive() || DamageAmount <= 0.f || StatusEffects->HasEffect(TagDefinitions::Status_Invincible)) return 0.f;
	if (DamageCauser && DamageCauser != this && GetTeamAttitudeTowards(*DamageCauser) == ETeamAttitude::Friendly) return 0.f;
	
	DamageAmount *= 1.f - FMath::Clamp(Statistics->GetFloat(TagDefinitions::Stats_DamageReduction), 0.f, 100.f) / 100.f;
	Resources->ModifyResource(TagDefinitions::Stats_Resources_Health, -DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AEstivalBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input) return;

	if (MoveAction)
	{
		Input->BindActionValueLambda(MoveAction, ETriggerEvent::Triggered, [this](const FInputActionValue& Value)
		{
			const FVector2D Axis = Value.Get<FVector2D>();
			DoMove(Axis.X, Axis.Y);
		});
	}
	
	for (const auto& InputPair : AbilityInputMap)
	{
		BindAbilityInput(Input, InputPair.Key, InputPair.Value);
	}

	if (StickAimAction)
	{
		Input->BindActionValueLambda(StickAimAction, ETriggerEvent::Triggered, [this](const FInputActionValue& Value)
		{
			Aim->SetAimDirection(InputToWorld(Value.Get<FVector2D>()));
		});

		Input->BindActionValueLambda(StickAimAction, ETriggerEvent::Completed, [this](const FInputActionValue&)
		{
			Aim->ClearAim();
		});
	}

	if (MouseAimAction)
	{
		Input->BindActionValueLambda(MouseAimAction, ETriggerEvent::Triggered, [this](const FInputActionValue&)
		{
			Aim->AimAtCursor();
		});
	}
}

void AEstivalBaseCharacter::DoMove(float AxisX, float AxisY)
{
	if (!CanMove()) return;

	AddMovementInput(InputToWorld(FVector2D(AxisX, AxisY)));
}

FVector AEstivalBaseCharacter::InputToWorld(const FVector2D Axis) const
{
	return FRotator(0.f, GetControlRotation().Yaw, 0.f).RotateVector(FVector(Axis.X, Axis.Y, 0.f));
}

void AEstivalBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	Resources->OnResourceChanged.AddDynamic(this, &AEstivalBaseCharacter::OnResourceChanged);
	StatusEffects->OnEffectAdded.AddDynamic(this, &AEstivalBaseCharacter::OnStatusEffectChanged);
	StatusEffects->OnEffectRemoved.AddDynamic(this, &AEstivalBaseCharacter::OnStatusEffectChanged);
	Statistics->OnStatChanged.AddDynamic(this, &AEstivalBaseCharacter::OnStatChanged);
	Loadout->OnAbilityActivated.AddDynamic(this, &AEstivalBaseCharacter::OnAbilityActivated);
	Loadout->OnAbilityCommitted.AddDynamic(this, &AEstivalBaseCharacter::OnAbilityCommitted);
	Loadout->OnAbilityEnded.AddDynamic(this, &AEstivalBaseCharacter::OnAbilityEnded);

	if (const auto& MoveSpeedTag = TagDefinitions::Stats_MoveSpeed; Statistics->HasStat(MoveSpeedTag)) 
		OnStatChanged(MoveSpeedTag, Statistics->GetFloat(MoveSpeedTag), 0.f);
	
	if (TeamId == FGenericTeamId::NoTeam) TeamId = DefaultTeam;
}

void AEstivalBaseCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	LookAt->SetLookDirection(Aim->GetAimDirection());
}

void AEstivalBaseCharacter::OnAbilityActivated(FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (!Ability || Ability->Facing != EAbilityFacing::WholeTime) return;

	FaceAim();
	SetActorTickEnabled(true);
}

void AEstivalBaseCharacter::OnAbilityCommitted(FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (Ability && Ability->Facing == EAbilityFacing::OnCommit) FaceAim();
}

void AEstivalBaseCharacter::OnAbilityEnded(FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (!Ability || Ability->Facing == EAbilityFacing::None) return;

	SetActorTickEnabled(false);
	LookAt->ClearLookTarget();
}

void AEstivalBaseCharacter::FaceAim()
{
	const FVector Direction = Aim->GetAimDirection();
	SetActorRotation(Direction.Rotation());
	LookAt->SetLookDirection(Direction);
}

void AEstivalBaseCharacter::OnResourceChanged(FGameplayTag Stat, float NewValue, float OldValue, float MaxValue)
{
	if (Stat.MatchesTagExact(TagDefinitions::Stats_Resources_Health))
	{
		if (!IsAlive()) return;
		if (NewValue <= 0.f && OldValue > 0.f) Die();
	}
}

void AEstivalBaseCharacter::BindAbilityInput(UEnhancedInputComponent* Input, const UInputAction* Action, FGameplayTag AbilityTag)
{
	if (!Input || !Action || !AbilityTag.IsValid()) return;
	
	Input->BindActionInstanceLambda(Action, ETriggerEvent::Started, [this, AbilityTag](const FInputActionInstance&)
		{
			if (CanUseAbilities()) Loadout->InputPressed(AbilityTag);
		});

	Input->BindActionInstanceLambda(Action, ETriggerEvent::Completed, [this, AbilityTag](const FInputActionInstance&)
		{
			Loadout->InputReleased(AbilityTag);
		});

	Input->BindActionInstanceLambda(Action, ETriggerEvent::Canceled, [this, AbilityTag](const FInputActionInstance&)
		{
			Loadout->InputReleased(AbilityTag);
		});
}

bool AEstivalBaseCharacter::TryActivateAbility(const FGameplayTag Slot)
{
	return CanUseAbilities() && Loadout->TryActivate(Slot);
}

bool AEstivalBaseCharacter::CanMove() const
{
	return IsAlive() && !StatusEffects->HasAnyEffect(MovementBlockingTags);
}

bool AEstivalBaseCharacter::CanUseAbilities() const
{
	return IsAlive() && !StatusEffects->HasAnyEffect(AbilityBlockingTags);
}

void AEstivalBaseCharacter::OnStatusEffectChanged(const FGameplayTag Tag)
{
	if (Tag.MatchesAny(AbilityBlockingTags) && !CanUseAbilities()) Loadout->Interrupt();
	if (!Tag.MatchesAny(MovementBlockingTags)) return;

	const bool bBlocked = StatusEffects->HasAnyEffect(MovementBlockingTags);
	LookAt->SetComponentTickEnabled(!bBlocked);
}

void AEstivalBaseCharacter::OnStatChanged(const FGameplayTag Stat, const float NewValue, float OldValue)
{
	if (Stat.MatchesTagExact(TagDefinitions::Stats_MoveSpeed)) GetCharacterMovement()->MaxWalkSpeed = NewValue;
}

void AEstivalBaseCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	
	Loadout->Interrupt();
 
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
 
	if (Controller) Controller->UnPossess();
	
	Resources->SetActive(false);
	OnDeath.Broadcast();
}
bool AEstivalBaseCharacter::TryRevive_Implementation(const float HealthPercent)
{
	if (IsAlive()) return false;
 
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	const auto& HealthTag = TagDefinitions::Stats_Resources_Health;
	const float Max = Resources->GetMax(HealthTag);
	Resources->SetCurrent(HealthTag, FMath::Max(1.f, Max * FMath::Clamp(HealthPercent, 0.f, 1.f)));
	
	if (PreviousController.Get())
	{
		if (!PreviousController->GetPawn()) PreviousController->Possess(this);
	}
	
	bIsDead = false;
	Resources->SetActive(true);
	OnRevived.Broadcast();
	return true;
}

bool AEstivalBaseCharacter::IsAlive() const
{
	return !bIsDead;
}