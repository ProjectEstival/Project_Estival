#include "DungeonGameMode.h"

#include "GenericTeamAgentInterface.h"
#include "AI/Companion/CompanionAIController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"
#include "Systems/Mike/Combat/TeamAttitudeSolver.h"
#include "Systems/Nephele/BaldrsLight.h"
#include "UI/HUDs/HUDBase.h"

namespace
{
	int32 CountRegisteredPlayers(const UGameInstance* GameInstance, const ULocalPlayerSessionDirectory* Directory)
	{
		if (!GameInstance || !Directory) return 0;
		int32 Count = 0;
		for (const ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
		{
			if (Directory->GetProfileId(LocalPlayer).IsValid()) ++Count;
		}
		return Count;
	}
}

void ADungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	FGenericTeamId::SetAttitudeSolver(&GetTeamBasedAttitude);
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ADungeonGameMode::InitializeBaseWidget));
}

void ADungeonGameMode::InitializeBaseWidget() const
{
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const bool bHasRegisteredPlayers = CountRegisteredPlayers(GI, Directory) > 0;
	const ULocalPlayer* FirstPlayer = GI ? GI->GetLocalPlayerByIndex(0) : nullptr;
	const APlayerController* FirstPlayerController = FirstPlayer ? FirstPlayer->GetPlayerController(GetWorld()) : nullptr;
	AHUDBase* FirstPlayerHUD = FirstPlayerController ? Cast<AHUDBase>(FirstPlayerController->GetHUD()) : nullptr;
	if (!IsValid(FirstPlayerHUD)) return;
	
	if (bHasRegisteredPlayers) FirstPlayerHUD->SetBaseWidget(CharacterSelectBaseWidgetClass, EManagedUIInputMode::GameAndUI, true);
	else FirstPlayerHUD->SetBaseWidget(InGameBaseWidgetClass, EManagedUIInputMode::GameOnly, true);
}

void ADungeonGameMode::HandleAllPlayersSelected(const TArray<FCharacterSelectPlayerChoice>& PlayerChoices)
{
	if (bHasHandledPlayerSelections) return;
	const UGameInstance* GI = GetGameInstance();
	if (const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr; CountRegisteredPlayers(GI, Directory) == 0) return;
	bHasHandledPlayerSelections = true;
	TArray<AActor*> BaldrsLights;
	UGameplayStatics::GetAllActorsOfClass(this, ABaldrsLight::StaticClass(), BaldrsLights);
	
	for (const FCharacterSelectPlayerChoice& Choice : PlayerChoices)
	{
		if (!Choice.CharacterClass) continue;
		APlayerController* PlayerController = IsValid(Choice.LocalPlayer) ? Choice.LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
		ACompanionAIController* AIController = nullptr;
		AController* Controller = PlayerController;
		if (IsValid(Choice.LocalPlayer))
		{
			if (!IsValid(PlayerController)) continue;
		}
		else
		{
			if (!CompanionAIControllerClass) continue;
			AIController = GetWorld()->SpawnActor<ACompanionAIController>(CompanionAIControllerClass, FTransform::Identity);
			if (!IsValid(AIController)) continue;
			Controller = AIController;
		}

		AActor* PlayerStart = FindPlayerStart(Controller);
		if (!IsValid(PlayerStart))
		{
			if (IsValid(AIController)) AIController->Destroy();
			continue;
		}
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AEstivalBaseCharacter* Character = GetWorld()->SpawnActor<AEstivalBaseCharacter>(Choice.CharacterClass, PlayerStart->GetActorTransform(), SpawnParameters);
		if (!IsValid(Character))
		{
			if (IsValid(AIController)) AIController->Destroy();
			continue;
		}
		
		if (ULoadoutComponent* LoadoutComponent = Character->GetLoadout())
		{
			LoadoutComponent->DefaultLoadout = Choice.Loadout;
			LoadoutComponent->SetLoadout(Choice.Loadout);
		}
		for (AActor* LightActor : BaldrsLights)
		{
			if (ABaldrsLight* BaldrsLight = Cast<ABaldrsLight>(LightActor)) BaldrsLight->AddPlayer(Character);
		}
		if (IsValid(PlayerController)) PlayerController->Possess(Character);
		else if (IsValid(AIController)) AIController->Possess(Character);
	}

	const ULocalPlayer* FirstPlayer = GI ? GI->GetLocalPlayerByIndex(0) : nullptr;
	const APlayerController* FirstPlayerController = FirstPlayer ? FirstPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (AHUDBase* FirstPlayerHUD = FirstPlayerController ? Cast<AHUDBase>(FirstPlayerController->GetHUD()) : nullptr; IsValid(FirstPlayerHUD) && InGameBaseWidgetClass) FirstPlayerHUD->SetBaseWidget(InGameBaseWidgetClass, EManagedUIInputMode::GameOnly, true);
}
