#include "LoadoutComponent.h"
#include "Systems/Mike/Combat/Abilities/AbilityContext.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "Loadout.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Systems/Mike/Stats/StatisticsComponent.h" 

ULoadoutComponent::ULoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
 
void ULoadoutComponent::BeginPlay()
{
	Super::BeginPlay();
	if (DefaultLoadout) SetLoadout(DefaultLoadout);
}
 
void ULoadoutComponent::SetLoadout(ULoadout* NewLoadout)
{
	if (NewLoadout == CurrentLoadout) return;
 
	ULoadout* OldLoadout = CurrentLoadout;
 
	if (UStatisticsComponent* Stats = ResolveStats())
	{
		if (NewLoadout && NewLoadout->BaseStats) Stats->SetStatCollection(NewLoadout->BaseStats);
	}
 
	CurrentLoadout = NewLoadout;
	CooldownEnds.Reset();
 
	OnLoadoutChanged.Broadcast(OldLoadout, NewLoadout);
}
 
UAbilityDefinition* ULoadoutComponent::GetAbility(const FGameplayTag Slot) const
{
	if (!CurrentLoadout) return nullptr;
	const TObjectPtr<UAbilityDefinition>* Ability = CurrentLoadout->Abilities.Find(Slot);
	return Ability ? Ability->Get() : nullptr;
}
 
float ULoadoutComponent::GetCooldownRemaining(const FGameplayTag Slot) const
{
	const float* ReadyAt = CooldownEnds.Find(Slot);
	const UWorld* World = GetWorld();
	if (!ReadyAt || !World) return 0;
 
	return FMath::Max(0.f, *ReadyAt - World->GetTimeSeconds());
}
 
bool ULoadoutComponent::IsOnCooldown(const FGameplayTag Slot) const
{
	return GetCooldownRemaining(Slot) > 0.f;
}

bool ULoadoutComponent::IsBusy() const
{
	if (const UWorld* World = GetWorld()) return RecoveryTime > World->GetTimeSeconds();
	return false;
}

bool ULoadoutComponent::TryActivate(const FGameplayTag Slot)
{
	UAbilityDefinition* Ability = GetAbility(Slot);
	AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
 
	if (!Ability || !Owner || !World) return false;
	if (IsOnCooldown(Slot) || IsBusy()) return false;
 
	const FAbilityContext Context = BuildContext(Ability);
	
	for (const auto& Pred : Ability->Predicates)
	{
		if (Pred && !Pred->IsSatisfied(Context)) return false;
	}
	
	if (bConsumesResources && Context.Resources)
	{
		for (const auto Cost : Ability->ResourceCosts)
		{
			if (!Cost.Key.IsValid()) continue;
			if (!Context.Resources->CanAfford(Cost.Key, Cost.Value)) return false;
		}
		for (const auto Cost : Ability->ResourceCosts)
		{
			if (!Cost.Key.IsValid()) continue;
			Context.Resources->TrySpend(Cost.Key, Cost.Value);
		}
	}
	const double Timestamp = World->GetTimeSeconds();
	CooldownEnds.Add(Slot, Timestamp + Ability->Cooldown);
	RecoveryTime = Timestamp + Ability->ExecuteDelay;
 
	if (Ability->Montage)
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			const float AnimDuration = Character->PlayAnimMontage(Ability->Montage);
			RecoveryTime = FMath::Max(RecoveryTime, AnimDuration + Timestamp);
		}
	}
	
	if (FMath::IsNearlyZero(Ability->ExecuteDelay))
	{
		ExecuteEffects(Context);
	}
	else
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateWeakLambda(this,
				[this, Ctx = Context]()
				{
					ExecuteEffects(Ctx);
				}),
			Ability->ExecuteDelay,
			false);
	}
 
	OnAbilityActivated.Broadcast(Slot, Ability);
	return true;
}
 
UStatisticsComponent* ULoadoutComponent::ResolveStats()
{
	if (!StatisticsComponent) StatisticsComponent = GetOwner()->FindComponentByClass<UStatisticsComponent>();
	return StatisticsComponent.Get();
}

UResourceComponent* ULoadoutComponent::ResolveResources()
{
	if (!ResourceComponent) ResourceComponent = GetOwner()->FindComponentByClass<UResourceComponent>();
	return ResourceComponent.Get();
}

FAbilityContext ULoadoutComponent::BuildContext(UAbilityDefinition* Ability)
{
	FAbilityContext Context;
	Context.Instigator = GetOwner();
	Context.Stats = ResolveStats();
	Context.Resources = ResolveResources();
	Context.Loadout = this;
	Context.Ability = Ability;
	return Context;
}
void ULoadoutComponent::ExecuteEffects(const FAbilityContext& Context)
{
	if (!Context.Ability) return;
	
	for (const TObjectPtr<UAbilityEffect>& Effect : Context.Ability->Effects)
	{
		if (Effect) Effect->Execute(Context);
	}
}