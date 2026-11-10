#include "AimComponent.h"
#include "GameFramework/PlayerController.h"

UAimComponent::UAimComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAimComponent::SetAimDirection(const FVector Direction)
{
	AimMode = EAimMode::Direction;
	Aim = Direction.GetSafeNormal2D();
}

void UAimComponent::SetAimLocation(const FVector Location)
{
	AimMode = EAimMode::Location;
	Aim = Location;
}

void UAimComponent::AimAtCursor()
{
	AimMode = EAimMode::Cursor;
}

void UAimComponent::ClearAim()
{
	AimMode = EAimMode::None;
}

FVector UAimComponent::GetAimDirection() const
{
	const AActor* Owner = GetOwner();
	FVector Direction = AimMode == EAimMode::Direction ? Aim : FVector::ZeroVector;

	if (FVector Point; FindAimPoint(Point)) Direction = (Point - Owner->GetActorLocation()).GetSafeNormal2D();

	return Direction.IsNearlyZero() ? Owner->GetActorForwardVector().GetSafeNormal2D() : Direction;
}

FVector UAimComponent::GetAimLocation(const float MaxRange) const
{
	const FVector From = GetOwner()->GetActorLocation();

	FVector Point;
	if (!FindAimPoint(Point)) return From + GetAimDirection() * MaxRange;
	if (MaxRange <= 0.f) return Point;

	const FVector Flat = FVector(Point.X - From.X, Point.Y - From.Y, 0.f).GetClampedToMaxSize(MaxRange);
	return FVector(From.X + Flat.X, From.Y + Flat.Y, Point.Z);
}

bool UAimComponent::FindAimPoint(FVector& OutPoint) const
{
	if (AimMode == EAimMode::Location)
	{
		OutPoint = Aim;
		return true;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* Player = Pawn ? Pawn->GetController<APlayerController>() : nullptr;

	FHitResult Hit;
	if (AimMode != EAimMode::Cursor || !Player || !Player->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) return false;

	OutPoint = Hit.Location;
	return true;
}
