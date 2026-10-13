// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/Core/Characters/A_CharacterBase.h"
#include "Systems/Thanos/Components/AC_LookAtComponent.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

AA_EstivalBaseCharacter::AA_EstivalBaseCharacter()
{
	// components do their own ticking
	PrimaryActorTick.bCanEverTick = false;
	
	LookAt = CreateDefaultSubobject<UAC_LookAtComponent>(TEXT("LookAt"));
	StatusEffects = CreateDefaultSubobject<UAC_StatusEffectComponent>(TEXT("StatusEffects"));
	Statistics = CreateDefaultSubobject<UStatisticsComponent>(TEXT("Statistics"));
	Resources = CreateDefaultSubobject<UResourceComponent>(TEXT("Resources"));

	// rotation is driven by the LookAt component
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

float AA_EstivalBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead() || DamageAmount <= 0.f) return 0.f;
	
	Resources->Modify(HealthTag, -DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AA_EstivalBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
}

void AA_EstivalBaseCharacter::DoMove(float AxisX, float AxisY)
{
	FRotator FlatRot = GetControlRotation();
	FlatRot.Pitch = 0.0f;

	AddMovementInput(FlatRot.RotateVector(FVector::ForwardVector), AxisX);
	AddMovementInput(FlatRot.RotateVector(FVector::RightVector), AxisY);
}

void AA_EstivalBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	Resources->OnResourceChanged.AddDynamic(this, &AA_EstivalBaseCharacter::OnResourceChanged);
}

void AA_EstivalBaseCharacter::OnResourceChanged(FGameplayTag Stat, float NewValue, float OldValue, float MaxValue)
{
	if (Stat != HealthTag || IsDead()) return;
	if (NewValue <= 0.f && OldValue > 0.f) Die();
}

void AA_EstivalBaseCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;
 
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
 
	if (Controller) Controller->UnPossess();
	
	Resources->SetActive(false);
	OnDeath.Broadcast();
}
bool AA_EstivalBaseCharacter::TryRevive(const float HealthPercent)
{
	if (!IsDead()) return false;
 
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	const float Max = Resources->GetMax(HealthTag);
	Resources->SetCurrent(HealthTag, FMath::Max(1.f, Max * FMath::Clamp(HealthPercent, 0.f, 1.f)));
	
	if (PreviousController.Get())
	{
		if (!PreviousController->GetPawn()) PreviousController->Possess(this);
	}
	
	Resources->SetActive(true);
	OnRevived.Broadcast();
	return true;
}

bool AA_EstivalBaseCharacter::IsDead() const
{
	return bIsDead;
}