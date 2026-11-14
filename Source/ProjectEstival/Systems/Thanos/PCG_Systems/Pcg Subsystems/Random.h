#pragma once

#include "CoreMinimal.h"

namespace Random {
	/**Weighted choice roll a number between 0 and the total weight,
	*then go through the list subtracting each weight until the roll fits in one.
	*Weight must not be empty.
	*/
	inline int32 PickWeightedIndex(const TArray<float>& Weights, FRandomStream& Random) {
		float TotalWeight = 0.0f;
		for (float Weight : Weights) {
			TotalWeight += FMath::Max(Weight, KINDA_SMALL_NUMBER);
		}

		float Roll = Random.FRandRange(0.0f, TotalWeight);
		for (int32 Index = 0; Index < Weights.Num(); Index++) {
			const float Weight = FMath::Max(Weights[Index], KINDA_SMALL_NUMBER);
			if (Roll <= Weight) {
				return Index;
			}
			Roll -= Weight;
		}
		return Weights.Num() - 1;
	}
}