#include "Systems/Core/Input/ControllerBase.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"

void AControllerBase::SetupInputComponent() {
	
	Super::SetupInputComponent();
	
	//just in case mr.Dok... Just    in    case
	if (!IsLocalPlayerController()) return;
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		for (UInputMappingContext* Context : DefaultMappingContexts) {
			if (Context) {
				Subsystem->AddMappingContext(Context, 0);
			}
		}
	}
}