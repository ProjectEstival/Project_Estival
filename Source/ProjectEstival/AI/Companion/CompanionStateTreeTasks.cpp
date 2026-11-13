// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Companion/CompanionStateTreeTasks.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "AI/Companion/CompanionAIController.h"
#include "Systems/Thanos/Components/AC_LookAtComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"
#include "AI/Companion/CompanionTactics.h"
#include "DrawDebugHelpers.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "StateTreeAsyncExecutionContext.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogCompanionAI, Log, All);

// ---------- Find Follow Target ----------

EStateTreeRunStatus FCompanionFindFollowTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// update on enter too, so the outputs are valid before the first tick
	UpdateTarget(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCompanionFindFollowTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UpdateTarget(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FCompanionFindFollowTargetTask::UpdateTarget(FInstanceDataType& InstanceData)
{
	// remember who we followed last tick, then start fresh
	APawn* OldTarget = InstanceData.FollowTarget;
	InstanceData.FollowTarget = nullptr;
	InstanceData.DistanceToTarget = 0.f;

	if (!InstanceData.Pawn)
	{
		return;
	}

	const FVector MyLocation = InstanceData.Pawn->GetActorLocation();
	APawn* NearestPawn = nullptr;
	double NearestDistance = TNumericLimits<double>::Max();
	double OldTargetDistance = -1.0;

	// a player is any pawn possessed by a PlayerController
	for (FConstPlayerControllerIterator It = InstanceData.Pawn->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			continue;
		}

		// real distance, not squared, so we can add a margin in cm below
		const double Distance = FVector::Dist(MyLocation, PlayerPawn->GetActorLocation());
		if (PlayerPawn == OldTarget)
		{
			OldTargetDistance = Distance;
		}

		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPawn = PlayerPawn;
		}
	}

	// stay with the old player unless the nearest one is clearly closer
	const bool bKeepOld = OldTargetDistance >= 0.0 && NearestDistance + InstanceData.SwitchMargin > OldTargetDistance;
	if (bKeepOld)
	{
		InstanceData.FollowTarget = OldTarget;
		InstanceData.DistanceToTarget = static_cast<float>(OldTargetDistance);
	}
	else if (NearestPawn)
	{
		InstanceData.FollowTarget = NearestPawn;
		InstanceData.DistanceToTarget = static_cast<float>(NearestDistance);
	}

	if (InstanceData.bDrawDebug && InstanceData.FollowTarget)
	{
		// green line: who I'm following
		DrawDebugLine(InstanceData.Pawn->GetWorld(), MyLocation, InstanceData.FollowTarget->GetActorLocation(), FColor::Green, false, -1.f, 0, 3.f);
	}

	// EQS cant read StateTree data so sharing the target through the controller
	if (ACompanionAIController* Companion = Cast<ACompanionAIController>(InstanceData.Pawn->GetController()))
	{
		Companion->SetFollowTarget(InstanceData.FollowTarget);
	}
}

// ---------- Look At ----------

EStateTreeRunStatus FCompanionLookAtTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	UpdateLook(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCompanionLookAtTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// targets move so update this every tick
	UpdateLook(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FCompanionLookAtTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UAC_LookAtComponent* LookAt = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<UAC_LookAtComponent>() : nullptr;
	if (LookAt)
	{
		// back to facing the movement direction
		LookAt->ClearLookTarget();
	}
}

void FCompanionLookAtTask::UpdateLook(const FInstanceDataType& InstanceData)
{
	UAC_LookAtComponent* LookAt = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<UAC_LookAtComponent>() : nullptr;
	if (!LookAt)
	{
		return;
	}

	const AActor* LookTarget = IsValid(InstanceData.PriorityTarget) ? InstanceData.PriorityTarget.Get() : InstanceData.Target.Get();
	if (LookTarget)
	{
		LookAt->SetLookAtLocation(LookTarget->GetActorLocation());
	}
}

// ---------- Find Threat ----------
EStateTreeRunStatus FCompanionFindThreatTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// remembering last tick's target before clearing the outputs
	AActor* OldTarget = InstanceData.AttackTarget;
	InstanceData.SeenEnemy = nullptr;
	InstanceData.AttackTarget = nullptr;
	InstanceData.Threats.Reset();

	const AEstivalBaseCharacter* Me = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	const UAIPerceptionComponent* Perception = InstanceData.AIController ? InstanceData.AIController->GetAIPerceptionComponent() : nullptr;
	if (!Me || !Perception)
	{
		return EStateTreeRunStatus::Running;
	}

	TArray<AActor*> SeenActors;
	Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	const double EngageRadiusSq = FMath::Square(InstanceData.EngageRadius);
	double NearestSeenSq = TNumericLimits<double>::Max();
	double NearestAttackSq = TNumericLimits<double>::Max();
	bool bOldTargetStillFighting = false;

	for (AActor* Actor : SeenActors)
	{
		// only living characters on a hostile team (never players or other bots!!)
		const AEstivalBaseCharacter* Enemy = Cast<AEstivalBaseCharacter>(Actor);
		if (!Enemy || !Enemy->IsAlive() || Me->GetTeamAttitudeTowards(*Enemy) != ETeamAttitude::Hostile)
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(Me->GetActorLocation(), Enemy->GetActorLocation());
		if (DistSq < NearestSeenSq)
		{
			NearestSeenSq = DistSq;
			InstanceData.SeenEnemy = Actor;
		}

		// "in the fight" = close to the player or close to me
		const bool bNearMe = DistSq < EngageRadiusSq;
		const bool bNearPlayer = InstanceData.FollowTarget
			&& FVector::DistSquared(InstanceData.FollowTarget->GetActorLocation(), Enemy->GetActorLocation()) < EngageRadiusSq;

		if (!bNearMe && !bNearPlayer)
		{
			continue;
		}
		
		// in the fight the target query may pick it
		InstanceData.Threats.Add(Actor);
		
		if (Actor == OldTarget)
		{
			bOldTargetStillFighting = true;
		}
		
		if (DistSq < NearestAttackSq)
		{
			NearestAttackSq = DistSq;
			InstanceData.AttackTarget = Actor;
		}
	}
	
	// stick to the target. companions should finish the fight they started
	if (bOldTargetStillFighting)
	{
		InstanceData.AttackTarget = OldTarget;
	}
	
	// EQS can't read StateTree data, so share the target and the threats through the controller
	ACompanionAIController* Companion = Cast<ACompanionAIController>(InstanceData.AIController.Get());
	if (Companion)
	{
		Companion->SetAttackTarget(InstanceData.AttackTarget);
		Companion->SetThreats(InstanceData.Threats);
	}
	
	// a new target found: fight the nearest for now and ask the style's target query for the best one
	const UCompanionTactics* Tactics = Companion ? Companion->GetTactics() : nullptr;
	if (InstanceData.AttackTarget && InstanceData.AttackTarget != OldTarget && Tactics && Tactics->TargetQuery)
	{
		StartTargetQuery(Context, InstanceData, Tactics->TargetQuery);
	}
		
	if (InstanceData.bDrawDebug && InstanceData.AttackTarget)
	{
		// line to who they fight, in the style's debug color (red if no style)
		const FColor LineColor = Tactics ? Tactics->DebugColor : FColor::Red;
		DrawDebugLine(Me->GetWorld(), Me->GetActorLocation(), InstanceData.AttackTarget->GetActorLocation(), LineColor, false, -1.f, 0, 3.f);
	}
	
	// global task: keeps running for the whole tree
	return EStateTreeRunStatus::Running;
}

void FCompanionFindThreatTask::StartTargetQuery(FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData, UEnvQuery* TargetQuery)
{
	// one question in the query at a time
	if (InstanceData.QueryId != INDEX_NONE)
	{
		if(UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(Context.GetOwner()))
		{
			QueryManager->AbortQuery(InstanceData.QueryId);
		}
		InstanceData.QueryId = INDEX_NONE;
	}

	// the pawn asks, so the generator and the tests can find this bot
	FEnvQueryRequest Request(TargetQuery, InstanceData.Pawn);
	InstanceData.QueryId = Request.Execute(EEnvQueryRunMode::SingleResult,
		// note: WeakContext stores a weak handle to this bot's tree inside the Lambda. Weak means it doesn't keep anything alive. 
		// It can only check later whether the tree is still there. 
		// mutable: because needs writing access to change it to usable later. 
		FQueryFinishedSignature::CreateLambda([WeakContext = Context.MakeWeakExecutionContext()](TSharedPtr<FEnvQueryResult> Result) mutable
		{
			// the answer comes a little later: find this task's data again, if the tree still runs
			const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
			FInstanceDataType* Data = StrongContext.GetInstanceDataPtr<FInstanceDataType>();
			if (!Data)
			{
				return;
			}

			Data->QueryId = INDEX_NONE;
			AActor* Best = (Result && Result->IsSuccessful()) ? Result->GetItemAsActor(0) : nullptr;

			// only if it's still in the fight. It may have died while EQS was thinking
			if (Best && Data->Threats.Contains(Best))
			{
				Data->AttackTarget = Best;
			}
		}));
}

// ---------- Use Ability ----------
EStateTreeRunStatus FCompanionUseAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	InstanceData.WaitTimer += DeltaTime;
	if (InstanceData.MaxWaitTime > 0.f && InstanceData.WaitTimer > InstanceData.MaxWaitTime)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	// abilities fire where the character faces so wait while Look At turns the companions
	if (IsValid(InstanceData.Target) && !IsFacing(Character, InstanceData.Target, InstanceData.AimAngle))
	{
		return EStateTreeRunStatus::Running;
	}
	
	// a set Slot always wins. An empty Slot means: try the bot style's list
	const bool bFired = InstanceData.Slot.IsValid() ? Character->TryActivateAbility(InstanceData.Slot) : TryTacticsAbilities(Character, InstanceData.Target);

	// ikf nothing fired yet try again next tick
	return bFired ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

bool FCompanionUseAbilityTask::IsFacing(const AActor* Me, const AActor* Target, const float AimAngle)
{
	const FVector Forward = Me->GetActorForwardVector().GetSafeNormal2D();
	const FVector ToTarget = (Target->GetActorLocation() - Me->GetActorLocation()).GetSafeNormal2D();
	
	return FVector::DotProduct(Forward, ToTarget) >= FMath::Cos(FMath::DegreesToRadians(AimAngle));
}

bool FCompanionUseAbilityTask::TryTacticsAbilities(AEstivalBaseCharacter* Me, const AActor* Target)
{
	// from the body to the brain where the style is stored
	const ACompanionAIController* Companion = Cast<ACompanionAIController>(Me->GetController());
	const UCompanionTactics* Tactics = Companion ? Companion->GetTactics() : nullptr;
	if (!Tactics)
	{
		return false;
	}

	// no target bound? Then only rows that start at 0 can match
	const double Distance = IsValid(Target) ? FVector::Dist(Me->GetActorLocation(), Target->GetActorLocation()) : 0.0;

	// from the top: the first row that is in range AND fires wins
	for (const FCompanionAbilityRule& Rule : Tactics->Abilities)
	{
		const bool bInRange = Distance >= Rule.MinRange && Distance <= Rule.MaxRange;
		if (bInRange && Me->TryActivateAbility(Rule.Slot))
		{
			return true;
		}
	}
	return false;
}

EStateTreeRunStatus FCompanionUseAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// make a fresh start every time the state is entered
	InstanceData.WaitTimer = 0.f;
	
	// making sure the slot is never empty: the bot style's list decides, so the bot needs a style
	if (!InstanceData.Slot.IsValid())
	{
		const ACompanionAIController* Companion = InstanceData.Pawn ? Cast<ACompanionAIController>(InstanceData.Pawn->GetController()) : nullptr;
		if (!Companion || !Companion->GetTactics())
		{
			UE_LOG(LogCompanionAI, Warning, TEXT("%s has no bot style (Tactics)"), *GetNameSafe(InstanceData.Pawn));
			return EStateTreeRunStatus::Failed;
		}
		return EStateTreeRunStatus::Running;
	}
	
	const ULoadoutComponent* Loadout = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<ULoadoutComponent>() : nullptr;
	
	// failing now instead of latr
	if (!Loadout || !Loadout->GetAbility(InstanceData.Slot))
	{
		UE_LOG(LogCompanionAI, Warning, TEXT("%s has nothing in slot %s"), *GetNameSafe(InstanceData.Pawn), *InstanceData.Slot.ToString());
		return EStateTreeRunStatus::Failed;
	}
	
	return EStateTreeRunStatus::Running;
}	


// ---------- Catch-Up Teleport ----------
EStateTreeRunStatus FCompanionCatchUpTeleportTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.Pawn || !InstanceData.FollowTarget)
	{
		return EStateTreeRunStatus::Running;
	}

	const float Distance = static_cast<float>(FVector::Dist(InstanceData.Pawn->GetActorLocation(), InstanceData.FollowTarget->GetActorLocation()));

	// close enough, nothing to watch
	if (Distance <= InstanceData.LeashRadius)
	{
		InstanceData.StuckTimer = 0.f;
		InstanceData.ClosestDistance = Distance;
		return EStateTreeRunStatus::Running;
	}

	// stuck: not getting at least 50 cm closer
	if (Distance < InstanceData.ClosestDistance - 50.f)
	{
		InstanceData.ClosestDistance = Distance;
		InstanceData.StuckTimer = 0.f;
	}
	else
	{
		InstanceData.StuckTimer += DeltaTime;
	}

	const bool bFarBehind = Distance > InstanceData.TeleportDistance;
	const bool bStuck = InstanceData.StuckTimer > InstanceData.StuckTime;

	// only cheat where nobody can see it
	if ((bFarBehind || bStuck) && !IsOnScreen(InstanceData.Pawn->GetWorld(), InstanceData.Pawn->GetActorLocation()))
	{
		if (TryTeleport(InstanceData))
		{
			InstanceData.StuckTimer = 0.f;
			InstanceData.ClosestDistance = TNumericLimits<float>::Max();
		}
	}

	// global task that keeps running for the whole tree
	return EStateTreeRunStatus::Running;
}

bool FCompanionCatchUpTeleportTask::IsOnScreen(const UWorld* World, const FVector& Location)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	
	if (!PlayerController)
	{
		return true;
	}

	FVector2D ScreenPosition;
	if (!UGameplayStatics::ProjectWorldToScreen(PlayerController, Location, ScreenPosition))
	{
		return false;
	}

	int32 Width = 0;
	int32 Height = 0;
	PlayerController->GetViewportSize(Width, Height);

	// a bot half visible at the edge should still count as on screen
	const float Margin = 100.f;
	return ScreenPosition.X > -Margin && ScreenPosition.X < Width + Margin
		&& ScreenPosition.Y > -Margin && ScreenPosition.Y < Height + Margin;
}

bool FCompanionCatchUpTeleportTask::TryTeleport(const FInstanceDataType& InstanceData)
{
	APawn* Pawn = InstanceData.Pawn;
	UWorld* World = Pawn->GetWorld();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	
	if (!NavSystem)
	{
		return false;
	}

	const FVector PlayerLocation = InstanceData.FollowTarget->GetActorLocation();
	const FVector Direction = (Pawn->GetActorLocation() - PlayerLocation).GetSafeNormal2D();
	const float Distance = static_cast<float>(FVector::Dist(Pawn->GetActorLocation(), PlayerLocation));

	// walk out from the player toward the bot's side and  land at the first point just a little bit off screen
	for (float Step = 600.f; Step < Distance; Step += 300.f)
	{
		FNavLocation NavLocation;
		// no floor here
		if (!NavSystem->ProjectPointToNavigation(PlayerLocation + Direction * Step, NavLocation))
		{
			continue; 
		}

		if (IsOnScreen(World, NavLocation.Location))
		{
			continue; 
		}

		// skip points behind a wall
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, PlayerLocation, NavLocation.Location + FVector(0.f, 0.f, 50.f), ECC_Visibility))
		{
			continue;
		}

		const FVector Destination = NavLocation.Location + FVector(0.f, 0.f, Pawn->GetSimpleCollisionHalfHeight());
		if (Pawn->TeleportTo(Destination, Pawn->GetActorRotation()))
		{
			// cancel the old path 
			// the tree picks a new move from here
			if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
			{
				AIController->StopMovement();
			}

			UE_LOG(LogCompanionAI, Log, TEXT("%s teleported to catch up"), *Pawn->GetName());
			return true;
		}
	}

	return false;
}

// ---------- Read Tactics ----------
EStateTreeRunStatus FCompanionReadTacticsTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// reads on enter too so the outputs are valid before the first tick
	Read(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCompanionReadTacticsTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// every tick because the style can change while playing
	Read(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FCompanionReadTacticsTask::Read(FInstanceDataType& InstanceData)
{
	const ACompanionAIController* Companion = Cast<ACompanionAIController>(InstanceData.AIController.Get());
	const UCompanionTactics* Tactics = Companion ? Companion->GetTactics() : nullptr;
	
	InstanceData.AttackSpotQuery = Tactics ? Tactics->AttackSpotQuery.Get() : nullptr;
}

void ACompanionAIController::NextTactics()
{
	if (TacticsPresets.IsEmpty())
	{
		return;
	}

	// IndexOfByKey gives INDEX_NONE (-1) if the current style isn't a preset, so we start at 0
	const int32 Current = TacticsPresets.IndexOfByKey(Tactics);

	// through SetTactics, so OnTacticsChanged fires and Blueprint hears about it
	SetTactics(TacticsPresets[(Current + 1) % TacticsPresets.Num()]);
}

#if !UE_BUILD_SHIPPING
// Estival.Bots.NextTactics: every bot switches to its next style
static void NextTacticsForAllBots(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		return;
	}

	for (TActorIterator<ACompanionAIController> It(World); It; ++It)
	{
		It->NextTactics();
	}
}

// registers the command when the game module loads
static FAutoConsoleCommandWithWorldAndArgs NextTacticsCommand(
	TEXT("Estival.Bots.NextTactics"),
	TEXT("Switches every companion bot to its next Bot Tactics preset."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&NextTacticsForAllBots));
#endif