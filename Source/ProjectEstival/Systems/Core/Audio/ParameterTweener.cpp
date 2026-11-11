#include "ParameterTweener.h"

FParameterTweener::~FParameterTweener()
{
    StopAll();
}

void FParameterTweener::Tween(UObject* Target, FName Parameter, float StartValue, float TargetValue, float Duration, TFunction<bool(float)> Setter)
{
    if (Parameter.IsNone()) return;

    Stop(Target, Parameter);

    if (Duration <= 0.f) return;

    FTween NewTween;
    NewTween.StartValue = StartValue;
    NewTween.TargetValue = TargetValue;
    NewTween.Duration = Duration;
    NewTween.Setter = MoveTemp(Setter);

    Tweens.Add(FTweenKey(Target, Parameter), MoveTemp(NewTween));

    if (!TickerHandle.IsValid())
    {
        TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FParameterTweener::Tick), 0.f);
    }
}

void FParameterTweener::Stop(UObject* Target, FName Parameter)
{
    Tweens.Remove(FTweenKey(Target, Parameter));

    if (Tweens.IsEmpty()) StopAll();
}

void FParameterTweener::StopAllForObject(UObject* Target)
{
    const TObjectKey<UObject> TargetKey(Target);

    for (auto It = Tweens.CreateIterator(); It; ++It)
    {
        if (It.Key().Target == TargetKey) It.RemoveCurrent();
    }

    if (Tweens.IsEmpty()) StopAll();
}

void FParameterTweener::StopAll()
{
    Tweens.Empty();

    if (TickerHandle.IsValid())
    {
        FTSTicker::RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }
}

bool FParameterTweener::Tick(float DeltaTime)
{
    for (auto It = Tweens.CreateIterator(); It; ++It)
    {
        FTween& Tween = It.Value();

        Tween.Elapsed += FMath::Max(DeltaTime, 0.f);

        const float Alpha = FMath::Clamp(Tween.Elapsed / Tween.Duration, 0.f, 1.f);
        const float Value = FMath::Lerp(Tween.StartValue, Tween.TargetValue, Alpha);

        if (!Tween.Setter(Value) || Alpha >= 1.f)
        {
            It.RemoveCurrent();
        }
    }

    if (Tweens.IsEmpty())
    {
        TickerHandle.Reset();
        return false;
    }

    return true;
}