#include "LoadoutComponent.h"
#include "Loadout.h"
#include "AimComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"
#include "Systems/Mike/Combat/Abilities/Effects/Tasks/AbilityTask.h"

ULoadoutComponent::ULoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
 
void ULoadoutComponent::BeginPlay()
{
	Super::BeginPlay();
	if (DefaultLoadout) SetLoadout(DefaultLoadout);
}

void ULoadoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Interrupt();

	for (const TArray<TObjectPtr<UAbilityTask>> Remaining = Tasks; UAbilityTask* Task : Remaining)
	{
		if (Task) Task->Cancel();
	}

	Super::EndPlay(EndPlayReason);
}

void ULoadoutComponent::SetLoadout(ULoadout* NewLoadout)
{
	if (NewLoadout == CurrentLoadout) return;
 
	Interrupt();
	ULoadout* OldLoadout = CurrentLoadout;

	if (UStatisticsComponent* Stats = GetStats())
	{
		if (NewLoadout && NewLoadout->BaseStats) Stats->SetStatCollection(NewLoadout->BaseStats);
	}
 
	CurrentLoadout = NewLoadout;

	TArray<FGameplayTag> OldSlots;
	Slots.GetKeys(OldSlots);
	for (const FGameplayTag& Slot : OldSlots) ClearFollowUp(Slot, false);
	Slots.Reset();
	
	if (NewLoadout)
	{
		for (const auto& Pair : NewLoadout->Abilities) EquipAbility(Pair.Key, Pair.Value);
	}
 
	OnLoadoutChanged.Broadcast(OldLoadout, NewLoadout);
}

void ULoadoutComponent::EquipAbility(const FGameplayTag Slot, UAbilityDefinition* Ability)
{
	if (!Slot.IsValid()) return;

	ClearFollowUp(Slot, false);
	FAbilitySlot& SlotData = Slots.FindOrAdd(Slot);

	SlotData.Ability = Ability;
	SlotData.CooldownStart = 0.f;
	SlotData.CooldownEnd = 0.f;

	OnSlotChanged.Broadcast(Slot, Ability);
}

UAbilityDefinition* ULoadoutComponent::GetAbility(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	return SlotData ? SlotData->GetCurrent() : nullptr;
}

bool ULoadoutComponent::HasFollowUp(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	return SlotData && SlotData->FollowUp;
}

bool ULoadoutComponent::IsOnCooldown(const FGameplayTag Slot) const
{
	return GetCooldownRemaining(Slot) > 0.f;
}
 
float ULoadoutComponent::GetCooldownRemaining(const FGameplayTag Slot) const
{
	const FAbilitySlot* SlotData = Slots.Find(Slot);
	const UWorld* World = GetWorld();

	if (!SlotData || !World || SlotData->FollowUp) return 0.f;
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

bool ULoadoutComponent::CanActivate(const FGameplayTag Slot)
{
	return GetBlockReason(Slot).IsEmpty();
}

bool ULoadoutComponent::TryActivate(const FGameplayTag Slot)
{
	if (const FString Reason = GetBlockReason(Slot); !Reason.IsEmpty())
	{
		if (bPrintDebug)
		{
			const FString Message = FString::Printf(TEXT("%s: %s didn't activate, %s"), *GetNameSafe(GetOwner()), *GetNameSafe(GetAbility(Slot)), *Reason);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, Message);
		}
		return false;
	}

	FAbilitySlot* SlotData = Slots.Find(Slot);
	UAbilityDefinition* Ability = SlotData->GetCurrent();

	if (Active)
	{
		Active->Cancel();

		SlotData = Slots.Find(Slot);
		if (!SlotData || SlotData->GetCurrent() != Ability) return false;
	}

	const bool bFromFollowUp = SlotData->FollowUp != nullptr;
	if (bFromFollowUp) GetWorld()->GetTimerManager().ClearTimer(SlotData->FollowUpTimer);

	Active = NewObject<UAbilityActivation>(this);
	OnAbilityActivated.Broadcast(Slot, Ability);
	Active->Start(this, Slot, Ability, bFromFollowUp);
	return true;
}

FString ULoadoutComponent::GetBlockReason(const FGameplayTag Slot)
{
	UAbilityDefinition* Ability = GetAbility(Slot);

	if (!Ability || !GetWorld()) return TEXT("no ability in this slot");
	if (IsOnCooldown(Slot)) return TEXT("on cooldown");
	if (Active && !Ability->bCanInterrupt) return TEXT("another ability is active");

	const FAbilityContext Context = MakeContext(Ability);

	for (const TObjectPtr<UAbilityPredicate>& Predicate : Ability->Predicates)
	{
		if (Predicate && !Predicate->IsSatisfied(Context)) return FString::Printf(TEXT("predicate %s failed"), *Predicate->GetClass()->GetName());
	}

	return CanAffordCosts(Ability) ? FString() : TEXT("can't afford the cost");
}

bool ULoadoutComponent::Interrupt()
{
	HeldSlots.Reset();
	if (!Active) return false;

	Active->Cancel();
	return true;
}

float ULoadoutComponent::GetCastProgress01() const
{
	if (!Active) return 0.f;

	for (const TObjectPtr<UAbilityTask>& Task : Tasks)
	{
		if (!Task || Task->GetActivation() != Active) continue;
		if (const float Progress = Task->GetProgress(); Progress >= 0.f) return FMath::Clamp(Progress, 0.f, 1.f);
	}

	return 0.f;
}

bool ULoadoutComponent::TryCommit(const FGameplayTag Slot, const UAbilityDefinition* Ability, const bool bFromFollowUp)
{
	if (!CanAffordCosts(Ability)) return false;
	SpendCosts(Ability);

	FAbilitySlot* SlotData = Slots.Find(Slot);
	if (!SlotData || SlotData->GetCurrent() != Ability) return true;

	if (!bFromFollowUp)
	{
		const float Now = GetWorld()->GetTimeSeconds();
		SlotData->CooldownStart = Now;
		SlotData->CooldownEnd = Now + Ability->Cooldown * GetCooldownMultiplier();
	}

	SetFollowUp(Slot, Ability->FollowUp);
	return true;
}

void ULoadoutComponent::HandleActivationEnded(UAbilityActivation* Activation, const bool bInterrupted)
{
	if (Activation != Active) return;
	Active = nullptr;

	const FGameplayTag Slot = Activation->GetSlot();
	UAbilityDefinition* Ability = Activation->GetAbility();

	if (HasFollowUp(Slot))
	{
		if (bInterrupted || !Activation->IsCommitted()) ClearFollowUp(Slot, true);
		else StartFollowUpWindow(Slot, Ability->FollowUpWindow);
	}

	if (bInterrupted) OnAbilityInterrupted.Broadcast(Slot, Ability);
	OnAbilityEnded.Broadcast(Slot, Ability);

	if (!bInterrupted && Ability->bRepeatWhileHeld) RepeatIfHeld(Slot);
}

bool ULoadoutComponent::InputPressed(const FGameplayTag Slot)
{
	if (!Slot.IsValid()) return false;
	HeldSlots.Add(Slot);

	if (Active && Active->GetSlot() == Slot)
	{
		ForEachTask(Active, [](UAbilityTask& Task)
		{
			Task.HandleInput(true);
		});
		
		return true;
	}

	return TryActivate(Slot);
}

void ULoadoutComponent::InputReleased(const FGameplayTag Slot)
{
	HeldSlots.Remove(Slot);

	if (Active && Active->GetSlot() == Slot)
	{
		ForEachTask(Active, [](UAbilityTask& Task)
		{
			Task.HandleInput(false);
		});
	}
}

void ULoadoutComponent::SendAbilityEvent(const FGameplayTag Event)
{
	if (!Event.IsValid()) return;

	if (Active) ForEachTask(Active, [Event](UAbilityTask& Task)
	{
		Task.HandleEvent(Event);
	});
	
	OnAbilityEvent.Broadcast(Event);
}

void ULoadoutComponent::RepeatIfHeld(const FGameplayTag Slot)
{
	UWorld* World = GetWorld();
	if (!World || !HeldSlots.Contains(Slot)) return;

	const FTimerDelegate Repeat = FTimerDelegate::CreateWeakLambda(this, [this, Slot]()
	{
		if (HeldSlots.Contains(Slot) && !IsBusy()) TryActivate(Slot);
	});

	FTimerHandle Handle;
	const float Delay = GetCooldownRemaining(Slot);
	if (Delay > 0.f) World->GetTimerManager().SetTimer(Handle, Repeat, Delay, false);
	else World->GetTimerManager().SetTimerForNextTick(Repeat);
}

void ULoadoutComponent::AddTask(UAbilityTask* Task)
{
	Tasks.Add(Task);
}

void ULoadoutComponent::RemoveTask(UAbilityTask* Task)
{
	Tasks.RemoveSingle(Task);
	if (Active && Task->GetActivation() == Active) Active->TryEnd();
}

void ULoadoutComponent::CancelTasks(const UAbilityActivation* Activation)
{
	ForEachTask(Activation, [](UAbilityTask& Task)
	{
		if (Task.GetLifetime() != EAbilityTaskLifetime::Background) Task.Cancel();
	});
}

bool ULoadoutComponent::HasTaskKeepingActive(const UAbilityActivation* Activation) const
{
	return Tasks.ContainsByPredicate([Activation](const UAbilityTask* Task)
	{
		return Task && Task->GetActivation() == Activation && Task->GetLifetime() == EAbilityTaskLifetime::KeepAbilityActive;
	});
}

void ULoadoutComponent::ForEachTask(const UAbilityActivation* Activation, const TFunctionRef<void(UAbilityTask&)> Function)
{
	const TArray<TObjectPtr<UAbilityTask>> Snapshot = Tasks;

	for (UAbilityTask* Task : Snapshot)
	{
		if (Task && Task->IsRunning() && Task->GetActivation() == Activation) Function(*Task);
	}
}

void ULoadoutComponent::SetFollowUp(const FGameplayTag Slot, UAbilityDefinition* FollowUp)
{
	FAbilitySlot* SlotData = Slots.Find(Slot);
	if (!SlotData || SlotData->FollowUp == FollowUp) return;

	if (const UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SlotData->FollowUpTimer);

	SlotData->FollowUp = FollowUp;
	OnSlotChanged.Broadcast(Slot, SlotData->GetCurrent());
}

void ULoadoutComponent::ClearFollowUp(const FGameplayTag Slot, const bool bBroadcast)
{
	FAbilitySlot* SlotData = Slots.Find(Slot);
	if (!SlotData) return;

	if (const UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SlotData->FollowUpTimer);
	if (!SlotData->FollowUp) return;

	SlotData->FollowUp = nullptr;
	if (bBroadcast) OnSlotChanged.Broadcast(Slot, SlotData->Ability);
}

void ULoadoutComponent::StartFollowUpWindow(const FGameplayTag Slot, const float Window)
{
	FAbilitySlot* SlotData = Slots.Find(Slot);
	UWorld* World = GetWorld();
	if (!SlotData || !World || Window <= 0.f) return;

	World->GetTimerManager().SetTimer(
		SlotData->FollowUpTimer,
		FTimerDelegate::CreateUObject(this, &ULoadoutComponent::ClearFollowUp, Slot, true),
		Window,
		false);
}

bool ULoadoutComponent::CanAffordCosts(const UAbilityDefinition* Ability)
{
	const UResourceComponent* Resources = GetResources();
	if (!bConsumesResources || !Resources) return true;
	
	for (const auto& Cost : Ability->ResourceCosts)
	{
		if (Cost.Key.IsValid() && !Resources->CanAfford(Cost.Key, Cost.Value)) return false;
	}
	return true;
}

void ULoadoutComponent::SpendCosts(const UAbilityDefinition* Ability)
{
	UResourceComponent* Resources = GetResources();
	if (!bConsumesResources || !Resources) return;
	
	for (const auto& Cost : Ability->ResourceCosts)
	{
		if (Cost.Key.IsValid()) Resources->ModifyResource(Cost.Key, -Cost.Value);
	}
}

float ULoadoutComponent::GetAttackSpeed()
{
	const UStatisticsComponent* Stats = GetStats();
	return FMath::Max(Stats ? Stats->GetFloat(TagDefinitions::Stats_AttackSpeed, 1.f) : 1.f, 0.1f);
}

float ULoadoutComponent::GetCooldownMultiplier()
{
	const UStatisticsComponent* Stats = GetStats();
	const float Reduction = Stats ? Stats->GetFloat(TagDefinitions::Stats_CooldownReduction, 0.f) : 0.f;
	return 1.f - FMath::Clamp(Reduction, 0.f, 100.f) / 100.f;
}

UStatisticsComponent* ULoadoutComponent::GetStats()
{
	if (!StatisticsComponent) StatisticsComponent = GetOwner()->FindComponentByClass<UStatisticsComponent>();
	return StatisticsComponent.Get();
}

UResourceComponent* ULoadoutComponent::GetResources()
{
	if (!ResourceComponent) ResourceComponent = GetOwner()->FindComponentByClass<UResourceComponent>();
	return ResourceComponent.Get();
}

UAimComponent* ULoadoutComponent::GetAim()
{
	if (!AimComponent) AimComponent = GetOwner()->FindComponentByClass<UAimComponent>();

	if (!AimComponent)
	{
		AimComponent = NewObject<UAimComponent>(GetOwner());
		AimComponent->RegisterComponent();
	}

	return AimComponent.Get();
}

FAbilityContext ULoadoutComponent::MakeContext(UAbilityDefinition* Ability, UAbilityActivation* Activation)
{
	AActor* Owner = GetOwner();

	FAbilityContext Context;
	Context.Instigator = Owner;
	Context.Stats = GetStats();
	Context.Resources = GetResources();
	Context.Loadout = this;
	Context.Ability = Ability;
	Context.World = GetWorld();
	Context.Activation = Activation;
	Context.Origin = Owner->GetActorLocation();
	Context.Direction = GetAim()->GetAimDirection();
	return Context;
}