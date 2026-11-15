#include "StatisticsComponent.h"
#include "StatDataAssets.h"
#include "TimerManager.h"
#include "Engine/World.h"
 
UStatisticsComponent::UStatisticsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}
 
void UStatisticsComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (DefaultStats) SetStatCollection(DefaultStats);
}
 
void UStatisticsComponent::SetStatCollection(const UStatDefinitionSet* Definitions)
{
	if (!Definitions) return;
	
	RegisteredStats.Reset();
	for (const TPair<FGameplayTag, FStatDefinition>& Pair : Definitions->Stats) RegisterStat(Pair.Key, Pair.Value);
}
 
void UStatisticsComponent::RegisterStat(const FGameplayTag StatTag, const FStatDefinition& Definition)
{
	if (!StatTag.IsValid()) return;
	
	FStat& Stat = Stats.FindOrAdd(StatTag);
	const float OldValue = Stat.CachedValue;
	
	Stat.SetupFromDef(Definition);
	RegisteredStats.Add(StatTag);
	
	if (!FMath::IsNearlyEqual(Stat.CachedValue, OldValue)) OnStatChanged.Broadcast(StatTag, Stat.CachedValue, OldValue);
}

bool UStatisticsComponent::HasStat(const FGameplayTag Stat) const
{
	return RegisteredStats.Contains(Stat);
}
 
float UStatisticsComponent::GetFloat(const FGameplayTag Stat, const float Fallback) const
{
	if (!HasStat(Stat)) return Fallback;
	const FStat* Found = Stats.Find(Stat);
	return Found? Found->CachedValue : Fallback;
}
 
int32 UStatisticsComponent::GetInt(const FGameplayTag Stat, const int32 Fallback) const
{
	return FMath::RoundToInt(GetFloat(Stat, Fallback));
}
 
void UStatisticsComponent::AddModifier(const FGameplayTag StatTag, const FStatModifier Modifier, const UObject* Source)
{
	if (!HasStat(StatTag)) return;
 
	FActiveStatModifier StatModifier;
	StatModifier.Modifier = Modifier;
	StatModifier.Source = Source;
	Stats[StatTag].Modifiers.Add(StatModifier);

	if (FStatChange Change; Recalculate(StatTag, Change)) BroadcastChanges({ Change });
}

void UStatisticsComponent::AddModifierSet(TMap<FGameplayTag, FStatModifier> Set, const UObject* Source)
{
	for (const auto& Mod : Set) AddModifier(Mod.Key, Mod.Value, Source);
}

void UStatisticsComponent::AddTimedModifierSet(TMap<FGameplayTag, FStatModifier> Set, const UObject* Source, const float Duration)
{
	if (!Source) return;

	RemoveAllFromSource(Source);
	AddModifierSet(Set, Source);

	UWorld* World = GetWorld();
	if (!World || Duration <= 0.f) return;

	const TWeakObjectPtr WeakSource(Source);
	FTimerHandle& Handle = SourceTimers.FindOrAdd(TObjectKey<UObject>(Source));

	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, WeakSource]()
	{
		if (const UObject* ExpiredSource = WeakSource.Get()) RemoveAllFromSource(ExpiredSource);
	}), Duration, false);
}

void UStatisticsComponent::ApplyModifierAsset(UStatModifierSet* Set, const UObject* Source)
{
	if (!Set) return;
	AddModifierSet(Set->Modifiers, Source);
}
 
int32 UStatisticsComponent::RemoveAllFromSource(const UObject* Source)
{
	if (!Source) return 0;
 
	if (FTimerHandle Handle; SourceTimers.RemoveAndCopyValue(TObjectKey<UObject>(Source), Handle))
	{
		if (const UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(Handle);
	}

	int32 TotalRemoved = 0;
	TArray<FStatChange> Changes;
 
	for (TPair<FGameplayTag, FStat>& Pair : Stats)
	{
		const int32 Removed = Pair.Value.Modifiers.RemoveAll([Source](const FActiveStatModifier& M)
		{
			return M.Source == Source;
		});
 
		if (Removed > 0)
		{
			TotalRemoved += Removed;
			FStatChange Change;
			if (Recalculate(Pair.Key, Change) && HasStat(Pair.Key)) Changes.Add(Change);
		}
	}
 
	BroadcastChanges(Changes);
	return TotalRemoved;
}
 
void UStatisticsComponent::ClearAllModifiers()
{
	if (const UWorld* World = GetWorld())
	{
		for (TPair<TObjectKey<UObject>, FTimerHandle>& Pair : SourceTimers) World->GetTimerManager().ClearTimer(Pair.Value);
	}
	SourceTimers.Reset();

	TArray<FStatChange> Changes;
 
	for (TPair<FGameplayTag, FStat>& Pair : Stats)
	{
		if (Pair.Value.Modifiers.Num() == 0) continue;
 
		Pair.Value.Modifiers.Reset();
 
		FStatChange Change;
		if (Recalculate(Pair.Key, Change) && HasStat(Pair.Key)) Changes.Add(Change);
	}
 
	BroadcastChanges(Changes);
}
 
bool UStatisticsComponent::Recalculate(const FGameplayTag& Tag, FStatChange& OutChange)
{
	if (!HasStat(Tag)) return false;
	
	FStat& Stat = Stats[Tag];
	const float Old = Stat.CachedValue;
	Stat.CachedValue = Stat.Calculate();
 
	if (FMath::IsNearlyEqual(Old, Stat.CachedValue)) return false;
 
	OutChange.Tag = Tag;
	OutChange.OldValue = Old;
	OutChange.NewValue = Stat.CachedValue;
	return true;
}
 
void UStatisticsComponent::BroadcastChanges(const TArray<FStatChange>& Changes) const
{
	for (const FStatChange& Change : Changes)
	{
		OnStatChanged.Broadcast(Change.Tag, Change.NewValue, Change.OldValue);
	}
}