#include "ResourceComponent.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

UResourceComponent::UResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true; 
}
void UResourceComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Statistics = GetOwner() ? GetOwner()->FindComponentByClass<UStatisticsComponent>() : nullptr;
	
	if (!Statistics)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: ResourceComponent needs a StatisticsComponent on the same actor."), *GetNameSafe(GetOwner()));
		return;
	}

	Statistics->OnStatChanged.AddDynamic(this, &UResourceComponent::HandleStatChanged);
}

void UResourceComponent::BeginPlay()
{
	Super::BeginPlay();
	
	bool bAnyRegen = false;
	
	for (const auto& Config : ResourceStats)
	{
		if (!Config.Key.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: a resource entry has no Stat tag."), *GetNameSafe(GetOwner()));
			continue;
		}

		if (Statistics && !Statistics->HasStat(Config.Key))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: resource stat '%s' is not registered in the stat component."), *GetNameSafe(GetOwner()), *Config.Key.ToString());
		}
		
		if (!FMath::IsNearlyZero(Config.Value.RegenPerSecond)) bAnyRegen = true;
	}

	SetComponentTickEnabled(bAnyRegen);
}

void UResourceComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!IsActive()) return;

	for (const auto& Config : ResourceStats)
	{
		if (FMath::IsNearlyZero(Config.Value.RegenPerSecond)) continue;
		Write(Config.Key, GetFloat(Config.Key) + Config.Value.RegenPerSecond * DeltaTime);
	}
}

const FResourceConfig* UResourceComponent::FindConfig(const FGameplayTag StatTag) const
{
	return ResourceStats.Find(StatTag);
}

bool UResourceComponent::HasResource(const FGameplayTag StatTag) const
{
	return FindConfig(StatTag) != nullptr;
}

float UResourceComponent::GetMax(const FGameplayTag StatTag) const
{
	return Statistics ? Statistics->GetFloat(StatTag) : 0.f;
}

float UResourceComponent::GetFloat(const FGameplayTag StatTag) const
{
	if (Statistics && !Statistics->HasStat(StatTag)) return 0.f;
	
	const FResourceConfig* Config = FindConfig(StatTag);
	if (!Config) return 0.f;
	
	if (const float* Found = Resources.Find(StatTag)) return *Found;
	return Config->bStartFull ? GetMax(StatTag) : 0.f;
}

int32 UResourceComponent::GetInt(const FGameplayTag StatTag) const
{
	return FMath::RoundToInt(GetFloat(StatTag));
}

float UResourceComponent::GetPercentage01(const FGameplayTag StatTag) const
{
	const float Max = GetMax(StatTag);
	return Max > 0.f ? GetFloat(StatTag) / Max : 0.f;
}

bool UResourceComponent::CanAfford(const FGameplayTag StatTag, const float Amount) const
{
	return GetFloat(StatTag) >= Amount;
}

float UResourceComponent::Modify(const FGameplayTag StatTag, const float Amount)
{
	if (!HasResource(StatTag)) return 0.f;
	return Write(StatTag, GetFloat(StatTag) + Amount);
}

bool UResourceComponent::TrySpend(const FGameplayTag StatTag, const float Amount)
{
	if (Amount <= 0.f) return true;
	if (!CanAfford(StatTag, Amount)) return false;

	Modify(StatTag, -Amount);
	return true;
}

void UResourceComponent::Fill(const FGameplayTag StatTag)
{
	if (HasResource(StatTag)) Write(StatTag, GetMax(StatTag));
}

void UResourceComponent::SetCurrent(const FGameplayTag StatTag, const float Value)
{
	if (HasResource(StatTag)) Write(StatTag, Value);
}

float UResourceComponent::Write(const FGameplayTag StatTag, float NewValue)
{
	const float Max = GetMax(StatTag);
	NewValue = FMath::Clamp(NewValue, 0.f, FMath::Max(Max, 0.f));
	
	const float Old = GetFloat(StatTag);
	if (FMath::IsNearlyEqual(Old, NewValue)) return 0.f;

	Resources.Add(StatTag, NewValue);
	OnResourceChanged.Broadcast(StatTag, NewValue, Old, Max);

	return NewValue - Old;
}

void UResourceComponent::HandleStatChanged(const FGameplayTag Stat, const float NewValue, const float OldValue)
{
	const FResourceConfig* Config = FindConfig(Stat);
	if (!Config) return;
	
	if (!Resources.Contains(Stat)) Resources.Add(Stat, Config->bStartFull ? OldValue : 0.f);
	float Target = Resources[Stat];
	
	if (Config->bStartFull && NewValue > OldValue) Target += NewValue - OldValue;
	Write(Stat, Target);
}