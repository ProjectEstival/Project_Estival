#include "DungeonGameMode.h"
#include "LevelManager.h"
#include "Engine/LocalPlayer.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Core/EstivalGameInstance.h"
#include "Systems/Core/Characters/PlayerBase.h"
#include "Systems/Mike/Combat/TeamAttitudeSolver.h"


ADungeonGameMode::ADungeonGameMode()
{
	Floors.SetNum(10);
	Floors[4].bRevivesParty = true;
	DefaultPawnClass = APlayerBase::StaticClass();
}

void ADungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	FGenericTeamId::SetAttitudeSolver(&RunAttitudes::GetTeamBasedAttitude);
}

void ADungeonGameMode::StartPlay()
{
	Super::StartPlay();

	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsWithInterface(this, ULevelManager::StaticClass(), Managers);
	
	if (Managers.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("DungeonGameMode: no level manager in this map, every floor starts at the PlayerStart."));
	}
	else LevelManager = Managers[0];

	SpawnParty();
	RequestFloor();
}

UClass* ADungeonGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (APlayerController* Player = Cast<APlayerController>(InController))
	{
		if (const TSubclassOf<APlayerBase> Hero = ChoosePlayerHero(Player)) return Hero;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

TSubclassOf<APlayerBase> ADungeonGameMode::ChoosePlayerHero_Implementation(APlayerController* Player)
{
	const TArray<TSubclassOf<APlayerBase>> Heroes = GetHeroes();
	const ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;
	if (Heroes.IsEmpty() || !LocalPlayer) return nullptr;

	return Heroes[LocalPlayer->GetLocalPlayerIndex() % Heroes.Num()];
}

void ADungeonGameMode::SpawnParty()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (APlayerBase* Hero = PlayerController ? Cast<APlayerBase>(PlayerController->GetPawn()) : nullptr) Party.Add(Hero);
	}
	
	const FTransform Start = GetPlayerStartTransform();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	while (Party.Num() < PartySize)
	{
		const TSubclassOf<APlayerBase> HeroClass = ChooseBotHero();
		APlayerBase* Bot = HeroClass ? GetWorld()->SpawnActor<APlayerBase>(HeroClass, Start, Params) : nullptr;
		if (!Bot) break;

		Bot->SpawnDefaultController();
		Party.Add(Bot);
	}

	for (APlayerBase* Member : Party)
	{
		Member->OnDeath.AddDynamic(this, &ADungeonGameMode::OnPartyMemberDied);
	}
}

void ADungeonGameMode::RequestFloor()
{
	Phase = EDungeonPhase::LoadingFloor;

	if (LevelManager)
	{
		ILevelManager::Execute_BuildFloor(LevelManager, GetFloor());
		return;
	}
	
	FloorReady(GetPlayerStartTransform(), nullptr);
}

void ADungeonGameMode::FloorReady(const FTransform& PartyStart, AEstivalBaseCharacter* Boss)
{
	if (Phase != EDungeonPhase::LoadingFloor) return;

	const FVector Right = PartyStart.GetRotation().GetRightVector();
	for (int32 i = 0; i < Party.Num(); i++)
	{
		const float Offset = (i - (Party.Num() - 1) / 2.f) * PartySpacing;
		if (Party[i]) Party[i]->TeleportTo(PartyStart.GetLocation() + Right * Offset, PartyStart.Rotator());
	}

	if (Boss)
	{
		CurrentBoss = Boss;
		CurrentBoss->OnDeath.AddUniqueDynamic(this, &ADungeonGameMode::OnBossDied);
	}
	else if (LevelManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonGameMode: floor %d has no boss, only the BossDefeated command can clear it."), GetFloor());
	}

	Phase = EDungeonPhase::Playing;
	OnFloorStarted.Broadcast(GetFloor());
}

void ADungeonGameMode::BossDefeated()
{
	OnBossDied();
}

void ADungeonGameMode::OnBossDied()
{
	if (Phase != EDungeonPhase::Playing) return;

	if (CurrentBoss)
	{
		CurrentBoss->OnDeath.RemoveDynamic(this, &ADungeonGameMode::OnBossDied);
		CurrentBoss = nullptr;
	}

	if (Floors[FloorIndex].bRevivesParty)
	{
		for (APlayerBase* Member : Party)
		{
			if (Member && !Member->IsAlive()) Member->TryRevive(PartyReviveHealthPercentage);
		}
	}

	if (FloorIndex == Floors.Num() - 1)
	{
		EndRun(ERunResult::Won);
		return;
	}

	Phase = EDungeonPhase::FloorCleared;
	OnFloorCleared.Broadcast(GetFloor());
}

void ADungeonGameMode::ContinueRun()
{
	if (Phase != EDungeonPhase::FloorCleared) return;

	FloorIndex++;
	RequestFloor();
}

void ADungeonGameMode::LeaveRun()
{
	if (Phase != EDungeonPhase::FloorCleared) return;
	EndRun(ERunResult::Left);
}

void ADungeonGameMode::OnPartyMemberDied()
{
	const bool bAnyoneAlive = Party.ContainsByPredicate([](const APlayerBase* Member)
	{
		return Member && Member->IsAlive();
	});
	
	if (!bAnyoneAlive) EndRun(ERunResult::Wiped);
}

void ADungeonGameMode::EndRun(const ERunResult Result)
{
	if (Phase == EDungeonPhase::RunOver) return;

	Phase = EDungeonPhase::RunOver;
	OnRunEnded.Broadcast(Result);

	FTimerHandle ReturnTimer;
	GetWorldTimerManager().SetTimer(ReturnTimer, this, &ADungeonGameMode::ReturnToHub, ReturnToHubDelay);
}

void ADungeonGameMode::ReturnToHub() const
{
	if (HubMap.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonGameMode: no HubMap set, staying here."));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, HubMap);
}

TArray<APlayerBase*> ADungeonGameMode::GetParty() const
{
	return TArray<APlayerBase*>(Party);
}

TArray<TSubclassOf<APlayerBase>> ADungeonGameMode::GetHeroes() const
{
	const UEstivalGameInstance* GameInstance = GetGameInstance<UEstivalGameInstance>();
	if (!GameInstance) return {};

	TArray<TSubclassOf<APlayerBase>> Heroes = GameInstance->GetHeroClasses().Array();
	Heroes.Remove(nullptr);
	return Heroes;
}

TSubclassOf<APlayerBase> ADungeonGameMode::ChooseBotHero() const
{
	const TArray<TSubclassOf<APlayerBase>> Heroes = GetHeroes();
	if (Heroes.IsEmpty()) return nullptr;

	TArray<TSubclassOf<APlayerBase>> Unused;
	for (const TSubclassOf<APlayerBase>& Hero : Heroes)
	{
		if (!Party.ContainsByPredicate([&Hero](const APlayerBase* Member) { return Member->GetClass() == Hero; })) 
			Unused.Add(Hero);
	}

	const TArray<TSubclassOf<APlayerBase>>& Choices = Unused.IsEmpty() ? Heroes : Unused;
	return Choices[FMath::RandHelper(Choices.Num())];
}

FTransform ADungeonGameMode::GetPlayerStartTransform()
{
	return FindPlayerStart(GetWorld()->GetFirstPlayerController())->GetActorTransform();
}