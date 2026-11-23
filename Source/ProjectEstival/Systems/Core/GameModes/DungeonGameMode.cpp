#include "DungeonGameMode.h"
#include "LevelManager.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "GenericTeamAgentInterface.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Core/EstivalGameInstance.h"
#include "Systems/Core/Characters/PlayerBase.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Systems/Dimitris/CurrencySubsystem.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "Systems/Mike/Combat/TeamAttitudeSolver.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"


ADungeonGameMode::ADungeonGameMode()
{
	Floors.SetNum(10);
	Floors[4].bRevivesParty = true;
	Floors[9].BossFragments = 3;
	DefaultPawnClass = APlayerBase::StaticClass();
}

void ADungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	FGenericTeamId::SetAttitudeSolver(&RunAttitudes::GetTeamBasedAttitude);
}

void ADungeonGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(SpawnHandle);
		if (UActorPoolSubsystem* Pool = World->GetSubsystem<UActorPoolSubsystem>()) Pool->OnActorAcquired.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ADungeonGameMode::StartPlay()
{
	Super::StartPlay();

	if (Floors.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonGameMode: the Floors list is empty, the run can't start."));
		return;
	}

	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsWithInterface(this, ULevelManager::StaticClass(), Managers);
	
	if (Managers.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("DungeonGameMode: no level manager in this map, every floor starts at the PlayerStart."));
	}
	else LevelManager = Managers[0];

	SpawnParty();

	SpawnHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ADungeonGameMode::OnActorAppeared));
	if (UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
	{
		Pool->OnActorAcquired.AddUObject(this, &ADungeonGameMode::OnActorAppeared);
	}
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
	PendingCharacters.Reset();
	RollFloorModifiers();

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
		if (!Party[i]) continue;

		const float Offset = (i - (Party.Num() - 1) / 2.f) * PartySpacing;
		if (!Party[i]->TeleportTo(PartyStart.GetLocation() + Right * Offset, PartyStart.Rotator()))
		{
			Party[i]->TeleportTo(PartyStart.GetLocation(), PartyStart.Rotator(), false, true);
		}
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

	for (AEstivalBaseCharacter* Character : TActorRange<AEstivalBaseCharacter>(GetWorld()))
	{
		ApplyFloorModifiers(Character);
	}

	Phase = EDungeonPhase::Playing;
	OnFloorStarted.Broadcast(GetFloor());
}

void ADungeonGameMode::RollFloorModifiers()
{
	const FDungeonFloor& Floor = Floors[FloorIndex];
	TArray<FFloorModifier> Pool = Floor.Modifiers;
	Pool.RemoveAll([](const FFloorModifier& Modifier) { return !Modifier.Set; });

	ActiveModifiers.Reset();
	while (ActiveModifiers.Num() < Floor.ModifierCount && !Pool.IsEmpty())
	{
		const FFloorModifier Picked = Pool[FMath::RandHelper(Pool.Num())];
		ActiveModifiers.Add(Picked);

		Pool.RemoveAll([&Picked](const FFloorModifier& Modifier)
		{
			return Modifier.Set == Picked.Set && Modifier.Target == Picked.Target;
		});
	}
}

void ADungeonGameMode::ApplyFloorModifiers(AEstivalBaseCharacter* Character)
{
	UStatisticsComponent* Stats = Character ? Character->GetStatistics() : nullptr;
	if (!Stats) return;

	Stats->RemoveAllFromSource(this);

	const APlayerBase* Hero = Cast<APlayerBase>(Character);
	const bool bInParty = Hero && Party.Contains(Hero);
	const APlayerBase* Leader = Party.IsEmpty() ? nullptr : Party[0].Get();
	const bool bIsEnemy = !bInParty && IsValid(Leader) && Leader->GetTeamAttitudeTowards(*Character) == ETeamAttitude::Hostile;

	for (const FFloorModifier& Modifier : ActiveModifiers)
	{
		const bool bTargeted = Modifier.Target == EFloorModifierTarget::Everyone
		|| (Modifier.Target == EFloorModifierTarget::Party ? bInParty : bIsEnemy);
		
		if (bTargeted) Stats->ApplyModifierAsset(Modifier.Set, this);
	}
}

void ADungeonGameMode::OnActorAppeared(AActor* Actor)
{
	if (ActiveModifiers.IsEmpty() || Phase == EDungeonPhase::LoadingFloor) return;

	AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(Actor);
	if (!Character) return;

	if (PendingCharacters.IsEmpty()) GetWorldTimerManager().SetTimerForNextTick(this, &ADungeonGameMode::ApplyPendingFloorModifiers);
	PendingCharacters.Add(Character);
}

void ADungeonGameMode::ApplyPendingFloorModifiers()
{
	const TSet<TWeakObjectPtr<AEstivalBaseCharacter>> Characters = PendingCharacters;
	PendingCharacters.Reset();

	for (const TWeakObjectPtr<AEstivalBaseCharacter>& Character : Characters)
	{
		ApplyFloorModifiers(Character.Get());
	}
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

	AwardFragments(Floors[FloorIndex].BossFragments);

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

void ADungeonGameMode::AwardFragments(const int32 Amount) const
{
	if (Amount <= 0) return;

	const UPlayerProfileSubsystem* Profiles = UGameInstance::GetSubsystem<UPlayerProfileSubsystem>(GetGameInstance());
	if (!Profiles) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ULocalPlayer* LocalPlayer = It->Get() ? It->Get()->GetLocalPlayer() : nullptr;
		UPlayerProfileSaveGame* Save = Profiles->GetProfile(LocalPlayer);
		
		if (!Save) continue;

		Save->Profile.UnspentFragments += Amount;
		Profiles->SaveProfile(LocalPlayer);
	}
}

void ADungeonGameMode::ContinueRun()
{
	if (Phase != EDungeonPhase::FloorCleared) return;

	FloorIndex++;
	RequestFloor();
}

void ADungeonGameMode::LeaveRun()
{
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
	
	if (const auto CurrencySystem = UGameInstance::GetSubsystem<UCurrencySubsystem>(GetGameInstance()))
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ULocalPlayer* LocalPlayer = It->Get() ? It->Get()->GetLocalPlayer() : nullptr;
			if (!LocalPlayer) continue;
			CurrencySystem->ClearRunCurrency(LocalPlayer);
		}
	}

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