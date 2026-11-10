#include "HitStopPayload.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

bool UHitStopPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const bool InstigatorSuccess = TryApplyHitStop(Context.Instigator, Duration);
	const bool TargetSuccess = TryApplyHitStop(Target, Duration);
	
	return InstigatorSuccess || TargetSuccess;
}

bool UHitStopPayload::TryApplyHitStop(AActor* TargetActor, const float Duration)
{
	if (!TargetActor || Duration <= 0.f) return false;
	
	if (FMath::IsNearlyZero(TargetActor->CustomTimeDilation)) return false;

	const UWorld* World = TargetActor->GetWorld();
	if (!World) return false;

	const float OriginalDilation = TargetActor->CustomTimeDilation;
	TargetActor->CustomTimeDilation = 0;

	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateWeakLambda(TargetActor, [TargetActor, OriginalDilation]()
		{
			TargetActor->CustomTimeDilation = OriginalDilation;
		}),
		Duration,
		false);

	return true;
}