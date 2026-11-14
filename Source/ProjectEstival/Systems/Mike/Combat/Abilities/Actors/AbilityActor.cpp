#include "AbilityActor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Indirect/SpawnAbilityActorEffect.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"
#include "Systems/Mike/Combat/Loadouts/RuntimeAbilities.h"

void AAbilityActor::InitializeAbilityActor(const FAbilityContext& InContext, const USpawnAbilityActorEffect* InSource)
{
	Context = InContext;
	Source = InSource;
	Hits = FTargetHitTracker();
	bExpired = false;

	SetOwner(Context.Instigator);
	SetInstigator(Cast<APawn>(Context.Instigator));

	if (IsValid(Context.Loadout)) Context.Loadout->OnAbilityEvent.AddUniqueDynamic(this, &AAbilityActor::HandleAbilityEvent);
}

void AAbilityActor::StartAbilityActor()
{
	OnStarted();
}

FGenericTeamId AAbilityActor::GetGenericTeamId() const
{
	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Context.Instigator.Get());
	return TeamAgent ? TeamAgent->GetGenericTeamId() : FGenericTeamId::NoTeam;
}

void AAbilityActor::LifeSpanExpired()
{
	Expire();
}

void AAbilityActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanUp();
	Super::EndPlay(EndPlayReason);
}

void AAbilityActor::OnReleased_Implementation()
{
	CleanUp();
}

void AAbilityActor::Expire()
{
	Finish(false);
}

void AAbilityActor::Cancel()
{
	Finish(true);
}

void AAbilityActor::HandleAbilityEvent(const FGameplayTag Event)
{
	if (bExpired) return;
	OnAbilityEvent(Event);

	const USpawnAbilityActorEffect* SpawnEffect = Source.Get();
	if (SpawnEffect && Event.MatchesTagExact(SpawnEffect->GetExpireOnEvent())) Expire();
}

void AAbilityActor::Finish(const bool bCancelled)
{
	if (bExpired) return;
	bExpired = true;

	if (const USpawnAbilityActorEffect* SpawnEffect = Source.Get(); SpawnEffect && !bCancelled)
	{
		SpawnEffect->RunExpireEffects(MakeActorContext());
	}

	OnExpired(bCancelled);

	CleanUp();

	if (UActorPoolSubsystem* Pool = UWorld::GetSubsystem<UActorPoolSubsystem>(GetWorld())) Pool->ReleaseActor(this);
	else Destroy();
}

void AAbilityActor::CleanUp()
{
	if (IsValid(Context.Loadout)) Context.Loadout->OnAbilityEvent.RemoveDynamic(this, &AAbilityActor::HandleAbilityEvent);
	if (UAbilityActivation* Activation = Context.Activation.Get()) Activation->OnInterrupted.RemoveAll(this);

	GetWorldTimerManager().ClearAllTimersForObject(this);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Context = FAbilityContext();
	Source = nullptr;
}

FAbilityContext AAbilityActor::MakeActorContext() const
{
	FAbilityContext ActorContext = Context;
	ActorContext.Origin = GetActorLocation();
	ActorContext.bFollowCaster = false;

	ActorContext.Activation = nullptr;

	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	if (!Forward.IsNearlyZero()) ActorContext.Direction = Forward;

	return ActorContext;
}

bool AAbilityActor::TryHit(AActor* Target)
{
	const USpawnAbilityActorEffect* SpawnEffect = Source.Get();
	if (!SpawnEffect || bExpired) return false;

	if (!SpawnEffect->TryApplyOnTarget(MakeActorContext(), Target, Hits)) return false;

	OnHitActor(Target);
	if (SpawnEffect->IsExhausted(Hits)) Expire();
	return true;
}
