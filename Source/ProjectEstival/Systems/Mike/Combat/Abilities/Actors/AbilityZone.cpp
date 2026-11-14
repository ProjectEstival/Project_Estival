#include "AbilityZone.h"
#include "Components/ArrowComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Indirect/SpawnZoneEffect.h"

AAbilityZone::AAbilityZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	if (Arrow)
	{
		Arrow->SetupAttachment(Root);
		Arrow->ArrowColor = FColor::Green;
	}
}

const USpawnZoneEffect* AAbilityZone::GetZoneEffect() const
{
	return Cast<USpawnZoneEffect>(GetSource());
}

void AAbilityZone::StartAbilityActor()
{
	Super::StartAbilityActor();
	LastHitTimes.Reset();

	const USpawnZoneEffect* ZoneEffect = GetZoneEffect();
	if (!ZoneEffect) return;

	FTimerManager& Timers = GetWorldTimerManager();

	if (ZoneEffect->TracksOccupants())
	{
		UpdateOccupants();
		if (IsExpired()) return;
		Timers.SetTimer(OccupantTimer, this, &AAbilityZone::UpdateOccupants, OccupantUpdateInterval, true);
	}

	if (ZoneEffect->HasTickEffects())
	{
		RunTick();
		if (IsExpired()) return;
		Timers.SetTimer(TickTimer, this, &AAbilityZone::RunTick, ZoneEffect->GetTickInterval(), true);
	}
}

void AAbilityZone::UpdateOccupants()
{
	const USpawnZoneEffect* ZoneEffect = GetZoneEffect();
	if (!ZoneEffect || IsExpired()) return;

	const float Now = GetWorld()->GetTimeSeconds();
	const float RehitInterval = ZoneEffect->GetRehitInterval();

	// Let actors be hit again once their rehit interval has passed
	if (RehitInterval > 0.f)
	{
		for (auto It = LastHitTimes.CreateIterator(); It; ++It)
		{
			AActor* Actor = It.Key().Get();
			if (Actor && Now - It.Value() < RehitInterval) continue;

			if (Actor) Hits.AlreadyHit.Remove(Actor);
			It.RemoveCurrent();
		}
	}

	const FAbilityContext ZoneContext = MakeActorContext();
	ZoneEffect->DrawDebug(ZoneContext, OccupantUpdateInterval);

	TArray<AActor*> Inside;
	ZoneEffect->CollectOccupants(ZoneContext, Inside);

	for (AActor* Actor : Inside)
	{
		const bool bWasTracked = Hits.AlreadyHit.Contains(Actor);

		TryHit(Actor);
		if (IsExpired()) return;

		if (RehitInterval > 0.f && !bWasTracked && Hits.AlreadyHit.Contains(Actor)) LastHitTimes.Add(Actor, Now);
	}
}

void AAbilityZone::RunTick() const
{
	if (const USpawnZoneEffect* ZoneEffect = GetZoneEffect(); ZoneEffect && !IsExpired()) ZoneEffect->RunTickEffects(MakeActorContext());
}
