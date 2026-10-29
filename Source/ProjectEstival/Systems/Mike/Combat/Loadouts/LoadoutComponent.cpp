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
 
	Interrupt();
	ULoadout* OldLoadout = CurrentLoadout;
 
	if (UStatisticsComponent* Stats = ResolveStats())
	{
		if (NewLoadout && NewLoadout->BaseStats) Stats->SetStatCollection(NewLoadout->BaseStats);
	}
 
	CurrentLoadout = NewLoadout;
	Slots.Reset();
	
	if (NewLoadout)
	{
		for (const auto& Pair : NewLoadout->Abilities) EquipAbility(Pair.Key, Pair.Value);
	}
 
	OnLoadoutChanged.Broadcast(OldLoadout, NewLoadout);
}
 
UAbilityDefinition* ULoadoutComponent::GetAbility(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	return SlotData ? SlotData->Ability.Get() : nullptr;
}
 
float ULoadoutComponent::GetCooldownRemaining(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	const UWorld* World = GetWorld();
	if (!SlotData || !World) return 0.f;
	
	return FMath::Max(0.f, SlotData->CooldownEnd - World->GetTimeSeconds());
}

float ULoadoutComponent::GetCooldownRemaining01(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	if (!SlotData) return 0.f;
	
	const float Duration = SlotData->CooldownEnd - SlotData->CooldownStart;
	return Duration > 0.f ? FMath::Clamp(GetCooldownRemaining(Slot) / Duration, 0.f, 1.f) : 0.f;
}

void ULoadoutComponent::ReduceCooldown(const FGameplayTag Slot, const float Seconds)
{
	FAbilitySlot* SlotData = Slots.Find(Slot);
	if (!SlotData || Seconds <= 0.f) return;
	
	SlotData->CooldownEnd -= Seconds;
}

void ULoadoutComponent::EquipAbility(const FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (!Slot.IsValid()) return;
	
	FAbilitySlot& SlotData = Slots.FindOrAdd(Slot);
	
	SlotData.Ability = Ability;
	SlotData.CooldownStart = 0.f;
	SlotData.CooldownEnd = 0.f;
	
	OnSlotChanged.Broadcast(Slot, Ability);
}
 
bool ULoadoutComponent::IsOnCooldown(const FGameplayTag Slot) const
{
	return GetCooldownRemaining(Slot) > 0.f;
}

float ULoadoutComponent::GetCastProgress01() const
{
	if (Active.Phase != EAbilityPhase::Windup || !Active.Ability) return 0.f;
	
	const UWorld* World = GetWorld();
	if (!World || Active.Ability->CastTime <= 0.f) return 1.f;
	
	const float Elapsed = World->GetTimeSeconds() - Active.PhaseStartTime;
	return FMath::Clamp(Elapsed / Active.Ability->CastTime, 0.f, 1.f);
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
	
	if (!CanAffordCosts(Ability)) return false;
	
	Active.Slot = Slot;
	Active.Ability = Ability;
	Active.Phase = EAbilityPhase::Windup;
	Active.PhaseStartTime = World->GetTimeSeconds();
	Active.MontageEndTime = Active.PhaseStartTime;
 
	if (Ability->Montage)
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			if (const float AnimDuration = Character->PlayAnimMontage(Ability->Montage); AnimDuration > 0.f)
			{
				Active.MontageEndTime += AnimDuration;
			}
		}
	}
	
	if (UStatisticsComponent* Stats = ResolveStats()) Stats->AddModifierSet(Ability->CastModifiers, Ability);
	
	OnAbilityActivated.Broadcast(Slot, Ability);
	
	if (FMath::IsNearlyZero(Ability->CastTime)) Commit();
	else World->GetTimerManager().SetTimer(Active.PhaseTimer, this, &ULoadoutComponent::Commit, Ability->CastTime, false);
	
	return true;
}

void ULoadoutComponent::Commit()
{
	if (Active.Phase != EAbilityPhase::Windup || !Active.Ability) return;
	
	UAbilityDefinition* Ability = Active.Ability;
	const FGameplayTag Slot = Active.Slot;
	const UWorld* World = GetWorld();
	
	if (!CanAffordCosts(Ability))
	{
		Interrupt();
		return;
	}
	
	RemoveCastingModifiers();
	SpendCosts(Ability);
	
	const float Now = World->GetTimeSeconds();
	FAbilitySlot* SlotData = Slots.Find(Slot);
	
	if (SlotData && SlotData->Ability == Ability)
	{
		SlotData->CooldownStart = Now;
		SlotData->CooldownEnd = Now + Ability->Cooldown;
	}
	
	Active.Phase = EAbilityPhase::Recovery;
	Active.PhaseStartTime = Now;
	
	OnAbilityCommitted.Broadcast(Slot, Ability);
	
	ExecuteEffects(BuildContext(Ability));
	
	const float Remaining = Active.MontageEndTime - Now;
	
	if (Remaining <= UE_KINDA_SMALL_NUMBER)
	{
		EndActive(false);
		return;
	}
	
	World->GetTimerManager().SetTimer(
		Active.PhaseTimer,
		FTimerDelegate::CreateUObject(this, &ULoadoutComponent::EndActive, false),
		Remaining,
		false);
}

bool ULoadoutComponent::Interrupt()
{
	if (Active.Phase == EAbilityPhase::Idle) return false;
	
	const bool bWasCasting = Active.Phase == EAbilityPhase::Windup;
	
	if (Active.Ability->Montage)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner())) Character->StopAnimMontage(Active.Ability->Montage);
	}
	
	EndActive(bWasCasting);
	return true;
}

void ULoadoutComponent::EndActive(const bool bInterrupted)
{
	if (Active.Phase == EAbilityPhase::Idle) return;
	
	if (const UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(Active.PhaseTimer);
	RemoveCastingModifiers();
	
	const FGameplayTag Slot = Active.Slot;
	UAbilityDefinition* Ability = Active.Ability;
	Active = FActiveAbility();
	
	if (bInterrupted) OnAbilityInterrupted.Broadcast(Slot, Ability);
	OnAbilityEnded.Broadcast(Slot, Ability);
}

void ULoadoutComponent::RemoveCastingModifiers()
{
	if (!Active.Ability) return;
	if (UStatisticsComponent* Stats = ResolveStats()) Stats->RemoveAllFromSource(Active.Ability);
}

bool ULoadoutComponent::CanAffordCosts(const UAbilityDefinition* Ability)
{
	const UResourceComponent* Resources = ResolveResources();
	if (!bConsumesResources || !Resources) return true;
	
	for (const auto& Cost : Ability->ResourceCosts)
	{
		if (Cost.Key.IsValid() && !Resources->CanAfford(Cost.Key, Cost.Value)) return false;
	}
	return true;
}

void ULoadoutComponent::SpendCosts(const UAbilityDefinition* Ability)
{
	UResourceComponent* Resources = ResolveResources();
	if (!bConsumesResources || !Resources) return;
	
	for (const auto& Cost : Ability->ResourceCosts)
	{
		if (Cost.Key.IsValid()) Resources->ModifyResource(Cost.Key, -Cost.Value);
	}
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
	Context.World = GetWorld();
	Context.Origin = Context.Instigator->GetActorTransform().GetLocation();
	Context.Direction = Context.Instigator->GetActorForwardVector().GetSafeNormal();
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