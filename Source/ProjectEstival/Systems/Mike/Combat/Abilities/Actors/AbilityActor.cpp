#include "AbilityActor.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Indirect/SpawnAbilityActorEffect.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void AAbilityActor::InitializeAbilityActor(const FAbilityContext& InContext, const USpawnAbilityActorEffect* InSource)
{
	Context = InContext;
	Source = InSource;

	if (IsValid(Context.Loadout)) Context.Loadout->OnAbilityEvent.AddDynamic(this, &AAbilityActor::HandleAbilityEvent);
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
	if (IsValid(Context.Loadout)) Context.Loadout->OnAbilityEvent.RemoveDynamic(this, &AAbilityActor::HandleAbilityEvent);
	Super::EndPlay(EndPlayReason);
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
	Destroy();
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
