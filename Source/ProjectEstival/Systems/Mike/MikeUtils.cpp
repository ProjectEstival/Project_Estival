#include "MikeUtils.h"

float UMikeUtils::RandomizeFloat(const float Val, float Percentage01)
{
	Percentage01 = FMath::Clamp(Percentage01, 0, 1);
	const float Offset = Percentage01 * Val;
	return FMath::FRandRange(Val - Offset, Val + Offset);
}