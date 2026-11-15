#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UObject/ObjectKey.h"

class FParameterTweener
{
public:
	~FParameterTweener();

	void Tween(UObject* Target, FName Parameter, float StartValue, float TargetValue, float Duration, TFunction<bool(float)> Setter);
	void Stop(UObject* Target, FName Parameter);
	void StopAllForObject(UObject* Target);
	void StopAll();

private:
	struct FTweenKey
	{
		TObjectKey<UObject> Target;
		FName Parameter;

		FTweenKey(UObject* InTarget, FName InParameter) : Target(InTarget), Parameter(InParameter) {}

		bool operator==(const FTweenKey& Other) const
		{
			return Target == Other.Target && Parameter == Other.Parameter;
		}

		friend uint32 GetTypeHash(const FTweenKey& Key)
		{
			return HashCombine(GetTypeHash(Key.Target), GetTypeHash(Key.Parameter));
		}
	};

	struct FTween
	{
		float StartValue = 0.f;
		float TargetValue = 0.f;
		float Duration = 0.f;
		float Elapsed = 0.f;

		TFunction<bool(float)> Setter;
	};

	TMap<FTweenKey, FTween> Tweens;
	FTSTicker::FDelegateHandle TickerHandle;

	bool Tick(float DeltaTime);
};