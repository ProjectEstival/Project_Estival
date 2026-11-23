// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "AI/Boss/BossRotation.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

DEFINE_LOG_CATEGORY(LogBossAI);

// Sets default values
ABossAIController::ABossAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	
	// don't start it on BeginPlay because StateTree can start twice, on BeginPlay and OnPossess. 
	StateTreeAI->SetStartLogicAutomatically(false);
	bStartAILogicOnPossess = false;
	
	// this keeps the controller where the pawn is, so location checks and EQS work as excpected.
	bAttachToPawn = true;
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// the rotation moves on when the body uses an ability so listen to its loadout
	if (const AEstivalBaseCharacter* Body = Cast<AEstivalBaseCharacter>(InPawn))
	{
		Body->GetLoadout()->OnAbilityCommitted.AddUniqueDynamic(this, &ABossAIController::HandleAbilityCommitted);
	}
	PickNextAbility();

	if (bAutoStartFight)
	{
		StartFight();
	}
}

void ABossAIController::OnUnPossess()
{
	// stop the tree while it still has its pawn otherwise StopLogic fails
	StateTreeAI->StopLogic(TEXT("Unpossessed"));
	
	if (const AEstivalBaseCharacter* Body = Cast<AEstivalBaseCharacter>(GetPawn()))
	{
		Body->GetLoadout()->OnAbilityCommitted.RemoveDynamic(this, &ABossAIController::HandleAbilityCommitted);
	}
	
	Super::OnUnPossess();
}

void ABossAIController::StartFight()
{
	if (bFightStarted || !GetPawn())
	{
		return;
	}
	
	bFightStarted = true;
	StateTreeAI->StartLogic();
	OnFightStarted.Broadcast();
	
	UE_LOG(LogBossAI, Log, TEXT("%s: the fight started"), *GetNameSafe(GetPawn()));
}


void ABossAIController::SetPhase(const int32 NewPhase)
{
	// phase only increase
	if (NewPhase <= Phase)
	{
		return;
	}
	
	const int32 OldPhase = Phase;
	Phase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase, OldPhase);
	
	UE_LOG(LogBossAI, Log, TEXT("%s: phase %d -> %d"), *GetNameSafe(GetPawn()), OldPhase, NewPhase);
	
	// a new phase plays its opening from the first step
	OpeningStep = 0;
	PickNextAbility();
}

// -------- Rotation -----------

void ABossAIController::PickNextAbility()
{
	NextAbility = FGameplayTag();
	NextPickedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	if (!Rotation || Rotation->Phases.IsEmpty())
	{
		return;
	}

	// phases past the last entry reuse the last one
	const FBossRotationPhase& Now = Rotation->Phases[FMath::Clamp(Phase, 0, Rotation->Phases.Num() - 1)];

	// Opening
	while (Now.Opening.IsValidIndex(OpeningStep))
	{
		const TArray<FGameplayTag>& Options = Now.Opening[OpeningStep++].AnyOf;
		if (!Options.IsEmpty())
		{
			NextAbility = Options[FMath::RandRange(0, Options.Num() - 1)];
			UE_LOG(LogBossAI, Log, TEXT("%s: next ability %s (opening)"), *GetNameSafe(GetPawn()), *NextAbility.ToString());
			return;
		}
	}

	// after the opening: weighted random. Not the same ability twice in a row
	TArray<TPair<FGameplayTag, float>> Candidates;
	float Total = 0.f;
	for (const TPair<FGameplayTag, float>& Entry : Now.Weights)
	{
		if (Entry.Value > 0.f && Entry.Key != LastAbility)
		{
			Candidates.Emplace(Entry.Key, Entry.Value);
			Total += Entry.Value;
		}
	}

	// only one usable ability: repeating it is better thanhaving the boss doing nothing
	if (Candidates.IsEmpty() && Now.Weights.FindRef(LastAbility) > 0.f)
	{
		Candidates.Emplace(LastAbility, Now.Weights.FindRef(LastAbility));
		Total = Now.Weights.FindRef(LastAbility);
	}

	float Roll = FMath::FRand() * Total;
	for (const TPair<FGameplayTag, float>& Candidate : Candidates)
	{
		Roll -= Candidate.Value;
		if (Roll <= 0.f)
		{
			NextAbility = Candidate.Key;
			break;
		}
	}

	UE_LOG(LogBossAI, Log, TEXT("%s: next ability %s"), *GetNameSafe(GetPawn()), *NextAbility.ToString());
}

bool ABossAIController::IsAbilityTurn() const
{
	// no rotation or no ability used yet: it is always the turn
	if (!Rotation || !LastAbility.IsValid())
	{
		return true;
	}

	const double Since = GetWorld()->GetTimeSeconds() - LastAbilityTime;
	return BasicsSinceAbility >= Rotation->BasicAttacksBetween || Since >= Rotation->MaxTimeBetween;
}

void ABossAIController::GetNextAbilityRange(float& OutMin, float& OutMax) const
{
	OutMin = 0.f;
	OutMax = 100000.f;

	if (const FBossAbilityRange* Range = Rotation ? Rotation->Ranges.Find(NextAbility) : nullptr)
	{
		OutMin = Range->Min;
		OutMax = Range->Max;
	}
}

bool ABossAIController::IsNextAbilityOverdue() const
{
	return Rotation && NextAbility.IsValid() && IsAbilityTurn()
		&& GetWorld()->GetTimeSeconds() - NextPickedTime > Rotation->Patience;
}

void ABossAIController::SkipNextAbility()
{
	UE_LOG(LogBossAI, Log, TEXT("%s: gave up on %s"), *GetNameSafe(GetPawn()), *NextAbility.ToString());

	// counts as the last one so the rotation doesn't pick it again right away
	LastAbility = NextAbility;
	PickNextAbility();
}

void ABossAIController::HandleAbilityCommitted(const FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (Slot == NextAbility)
	{
		LastAbility = Slot;
		LastAbilityTime = GetWorld()->GetTimeSeconds();
		BasicsSinceAbility = 0;
		PickNextAbility();
	}
	else if (Rotation && Slot == Rotation->BasicAttack)
	{
		++BasicsSinceAbility;
	}
}

