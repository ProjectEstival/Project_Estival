#include "MoveCasterTask.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

UMoveCasterTask::UMoveCasterTask()
{
	Lifetime = EAbilityTaskLifetime::KeepAbilityActive;
}

void UMoveCasterTask::OnStart_Implementation()
{
	const FAbilityContext Now = GetContext();
	const ACharacter* Character = Cast<ACharacter>(Now.Instigator);

	if (Duration <= 0.f && IsValid(Now.Instigator))
	{
		Teleport(Now.Instigator, FindDestination(Now));
		Arrive();
	}
	else if (IsValid(Character)) StartMoving(Character, FindDestination(Now));
	else Finish();
}

void UMoveCasterTask::OnEnd_Implementation(const bool bCancelled)
{
	const ACharacter* Character = Cast<ACharacter>(GetContext().Instigator);
	if (bCancelled && IsValid(Character)) Character->GetCharacterMovement()->RemoveRootMotionSourceByID(MoveId);
}

FVector UMoveCasterTask::FindDestination(const FAbilityContext& Now) const
{
	if (Direction == EMoveCasterDirection::ToOrigin) return Now.Origin;

	const AActor* Caster = Now.Instigator;
	FVector Way = Now.Direction;

	if (Direction == EMoveCasterDirection::Movement)
	{
		const APawn* Pawn = Cast<APawn>(Caster);
		Way = Pawn ? Pawn->GetLastMovementInputVector().GetSafeNormal2D() : FVector::ZeroVector;
		if (Way.IsNearlyZero()) Way = Caster->GetVelocity().GetSafeNormal2D();
	}

	if (Direction == EMoveCasterDirection::Facing || Way.IsNearlyZero()) Way = Caster->GetActorForwardVector().GetSafeNormal2D();
	return Caster->GetActorLocation() + Way * Distance;
}

void UMoveCasterTask::Teleport(AActor* Caster, FVector To) const
{
	const FVector From = Caster->GetActorLocation();

	if (!bThroughWalls)
	{
		// Ignore characters, only real walls stop the teleport
		FCollisionResponseParams Walls;
		Walls.CollisionResponse.SetResponse(ECC_Pawn, ECR_Ignore);

		FHitResult Wall;
		const FCollisionQueryParams Params(SCENE_QUERY_STAT(MoveCaster), false, Caster);
		const FCollisionShape Body = FCollisionShape::MakeSphere(Caster->GetSimpleCollisionRadius());

		if (GetWorld()->SweepSingleByChannel(Wall, From, FVector(To.X, To.Y, From.Z), FQuat::Identity, ECC_Pawn, Body, Params, Walls)) To = Wall.Location;
	}
	
	for (int32 Step = 10; Step > 0; --Step) // Land where the caster fits
	{
		if (Caster->TeleportTo(FMath::Lerp(From, To, Step / 10.f), Caster->GetActorRotation())) return;
	}
}

void UMoveCasterTask::StartMoving(const ACharacter* Character, const FVector& To)
{
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	const FVector Flat = (To - Character->GetActorLocation()) * FVector(1.f, 1.f, 0.f);

	const TSharedPtr<FRootMotionSource_ConstantForce> Move = MakeShared<FRootMotionSource_ConstantForce>();
	Move->AccumulateMode = ERootMotionAccumulateMode::Override;
	Move->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	Move->Force = Flat / Duration;
	Move->Duration = Duration;
	Move->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
	Move->FinishVelocityParams.ClampVelocity = Movement->GetMaxSpeed();
	MoveId = Movement->ApplyRootMotionSource(Move);

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UMoveCasterTask::Arrive, Duration, false);
}

void UMoveCasterTask::Arrive()
{
	FAbilityContext Arrived = GetContext();
	if (IsValid(Arrived.Instigator)) Arrived.Origin = Arrived.Instigator->GetActorLocation();

	ExecuteAll(ArriveEffects, Arrived);
	Finish();
}
