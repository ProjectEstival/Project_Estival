#include "StatisticsComponent.h"
#include "StatDataAssets.h"
 
UStatisticsComponent::UStatisticsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}
 
void UStatisticsComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (DefaultStats) InitializeFromDefinitions(DefaultStats);
}
 
void UStatisticsComponent::InitializeFromDefinitions(const UStatDefinitionSet* Definitions)
{
	if (!Definitions) return;
	
	for (const TPair<FGameplayTag, FStatDefinition>& Pair : Definitions->Stats)
		RegisterStat(Pair.Key, Pair.Value);
}
 
void UStatisticsComponent::RegisterStat(const FGameplayTag StatTag, const FStatDefinition& Definition)
{
	if (!StatTag.IsValid()) return;
	
	FStat Stat;
	Stat.SetupFromDef(Definition);
	Stats.Add(StatTag, Stat);
}

bool UStatisticsComponent::HasStat(const FGameplayTag Stat) const
{
	return Stats.Contains(Stat);
}
 
float UStatisticsComponent::GetFloat(const FGameplayTag Stat, const float Fallback) const
{
	const FStat* Found = Stats.Find(Stat);
	return Found? Found->CachedValue : Fallback;
}
 
int32 UStatisticsComponent::GetInt(const FGameplayTag Stat, const int32 Fallback) const
{
	const FStat* Found = Stats.Find(Stat);
	return Found ? FMath::RoundToInt(Found->CachedValue) : Fallback;
}
 
void UStatisticsComponent::AddModifier(const FGameplayTag StatTag, const FStatModifier Modifier, UObject* Source)
{
	if (!HasStat(StatTag)) return;
 
	FActiveStatModifier StatModifier;
	StatModifier.Modifier = Modifier;
	StatModifier.Source = Source;
	Stats[StatTag].Modifiers.Add(StatModifier);

	if (FStatChange Change; Recalculate(StatTag, Change)) BroadcastChanges({ Change });
}

void UStatisticsComponent::AddModifierSet(TMap<FGameplayTag, FStatModifier> Set, UObject* Source)
{
	for (const auto& Mod : Set) AddModifier(Mod.Key, Mod.Value, Source);
}

void UStatisticsComponent::ApplyModifierAsset(UStatModifierSet* Set, UObject* Source)
{
	if (!Set) return;
	AddModifierSet(Set->Modifiers, Source);
}
 
int32 UStatisticsComponent::RemoveAllFromSource(UObject* Source)
{
	if (!Source) return 0;
 
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
			if (Recalculate(Pair.Key, Change)) Changes.Add(Change);
		}
	}
 
	BroadcastChanges(Changes);
	return TotalRemoved;
}
 
void UStatisticsComponent::ClearAllModifiers()
{
	TArray<FStatChange> Changes;
 
	for (TPair<FGameplayTag, FStat>& Pair : Stats)
	{
		if (Pair.Value.Modifiers.Num() == 0) continue;
 
		Pair.Value.Modifiers.Reset();
 
		FStatChange Change;
		if (Recalculate(Pair.Key, Change)) Changes.Add(Change);
	}
 
	BroadcastChanges(Changes);
}
 
bool UStatisticsComponent::Recalculate(const FGameplayTag& Tag, FStatChange& OutChange)
{
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