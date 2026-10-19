#include "Systems/Core/Input/ControllerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"

void AControllerBase::SetupInputComponent() {
	
	Super::SetupInputComponent();
	
	//just in case mistar Dok.... Just   in   case
	if (!IsLocalPlayerController()) return; {
		
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
			if (DefaultMappingContext) {
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		} 
	}
	
	//move as long as the button is pressed
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AControllerBase::Move);
		}
	}
}

void AControllerBase::Move(const FInputActionValue& Value) {
	
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;
	
	// X = <-    Y = ->
	const FVector2D Axis = Value.Get<FVector2D>();
	
	ControlledPawn->AddMovementInput(FVector::ForwardVector, Axis.X);
	ControlledPawn->AddMovementInput(FVector::RightVector, Axis.Y);
}