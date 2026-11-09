#include "PlayFeedbackPayload.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Systems/Core/Audio/AudioManager.h"

bool UPlayFeedbackPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target)) return false;

	USceneComponent* Part = FindPart(Target);
	UAudioManager* Audio = UGameInstance::GetSubsystem<UAudioManager>(Target->GetGameInstance());

	if (bAttach && !bAtOrigin)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(Visual, Part, Socket, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		if (Audio) Audio->PlayOneShotAttached(Sound, Part);
	}
	else
	{
		const FVector Where = bAtOrigin ? Context.Origin : Part->GetSocketLocation(Socket);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target, Visual, Where, Context.Direction.Rotation());
		if (Audio) Audio->PlayOneShotAtLocation(Sound, Where);
	}

	if (CameraShake)
	{
		if (APlayerController* FirstPlayer = Target->GetWorld()->GetFirstPlayerController()) 
			FirstPlayer->ClientStartCameraShake(CameraShake, ShakeStrength);
	}

	return true;
}

USceneComponent* UPlayFeedbackPayload::FindPart(const AActor* Receiver) const
{
	TInlineComponentArray<USceneComponent*> Parts(Receiver);
	for (USceneComponent* Part : Parts)
	{
		if (Part->DoesSocketExist(Socket)) return Part;
	}
	return Receiver->GetRootComponent();
}
