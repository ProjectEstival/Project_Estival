#include "PlayerBase.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/DecalComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"

APlayerBase::APlayerBase()
{
	ResurrectionComponent = CreateDefaultSubobject<UReviveComponent>(TEXT("Resurrection"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	ColorRing = CreateDefaultSubobject<UDecalComponent>(TEXT("ColorRing"));
	ColorRing->SetupAttachment(GetMesh());
	ColorRing->SetRelativeLocation(FVector::ZeroVector);
	ColorRing->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ColorRing->DecalSize = FVector(64.0f);
	GetMesh()->bReceivesDecals = false;
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input) return;
	
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerBase::StartInteract);
		Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &APlayerBase::StopInteract);
		Input->BindAction(InteractAction, ETriggerEvent::Canceled, this, &APlayerBase::StopInteract);
	}
}

void APlayerBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (ColorRing && GetMesh())
	{
		if (ColorRing->GetAttachParent() != GetMesh()) ColorRing->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
		ColorRing->SetRelativeLocation(FVector::ZeroVector);
		ColorRing->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		ColorRing->DecalSize = FVector(64.0f);
	}
}

void APlayerBase::SetPlayerColor(const FLinearColor& Color)
{
	if (!ColorRing) return;
	if (!ColorRingMaterial) ColorRingMaterial = ColorRing->CreateDynamicMaterialInstance();
	ColorRing->SetDecalColor(Color);
	if (ColorRingMaterial) ColorRingMaterial->SetVectorParameterValue(TEXT("DecalColor"), Color);
}

void APlayerBase::StartInteract()
{
	InteractionComponent->StartInteract();
}

void APlayerBase::StopInteract()
{
	InteractionComponent->StopInteract();
}
