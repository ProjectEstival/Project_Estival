#include "AbilityTask.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/LatentActionManager.h"
#include "GameFramework/Pawn.h"
#include "UObject/UnrealType.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void UAbilityTask::Execute_Implementation(const FAbilityContext& InContext) const
{
	ULoadoutComponent* Loadout = InContext.Loadout;
	if (!IsValid(Loadout)) return;

	UAbilityTask* Run = NewObject<UAbilityTask>(Loadout, GetClass(), NAME_None, RF_Transient);

	for (TFieldIterator<FProperty> It(GetClass()); It; ++It)
	{
		if (!It->HasAnyPropertyFlags(CPF_Transient)) It->CopyCompleteValue_InContainer(Run, this);
	}

	Run->Start(InContext);
}

UWorld* UAbilityTask::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : Context.World.Get();
}

void UAbilityTask::Start(const FAbilityContext& InContext)
{
	Context = InContext;
	StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bRunning = true;

	const UAbilityActivation* Activation = GetActivation();
	if (!Activation || !Activation->IsActive()) Lifetime = EAbilityTaskLifetime::Background;

	Context.Loadout->AddTask(this);
	SpawnIndicator();
	OnStart();
}

void UAbilityTask::Finish()
{
	End(false);
}

void UAbilityTask::Cancel()
{
	End(true);
}

void UAbilityTask::End(const bool bCancelled)
{
	if (!bRunning) return;
	bRunning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
		World->GetLatentActionManager().RemoveActionsForObject(this);
	}

	if (IsValid(SpawnedIndicator)) SpawnedIndicator->Destroy();
	SpawnedIndicator = nullptr;

	OnEnd(bCancelled);
	if (IsValid(Context.Loadout)) Context.Loadout->RemoveTask(this);
}

void UAbilityTask::HandleInput(const bool bPressed)
{
	if (!bRunning) return;

	if (bPressed) OnInputPressed();
	else OnInputReleased();
}

void UAbilityTask::HandleEvent(const FGameplayTag Event)
{
	if (bRunning) OnAbilityEvent(Event);
}

FAbilityContext UAbilityTask::GetContext() const
{
	FAbilityContext Current = Context;
	if (!Current.bFollowCaster || !IsValid(Current.Instigator) || !IsValid(Current.Loadout)) return Current;

	Current.Origin = Current.Instigator->GetActorLocation();
	Current.Direction = Current.Loadout->GetAimDirection();
	return Current;
}

void UAbilityTask::RunEffects(const TArray<UAbilityEffect*>& Effects) const
{
	ExecuteAll(Effects, GetContext());
}

bool UAbilityTask::IsInputHeld() const
{
	const UAbilityActivation* Activation = GetActivation();
	return Activation && Activation->IsActive() && Context.Loadout->IsSlotHeld(Activation->GetSlot());
}

float UAbilityTask::GetElapsedTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() - StartTime : 0.f;
}

bool UAbilityTask::CommitAbility() const
{
	UAbilityActivation* Activation = GetActivation();
	return !Activation || !Activation->IsActive() || Activation->Commit();
}

void UAbilityTask::SpawnIndicator()
{
	UWorld* World = GetWorld();
	if (!Indicator || !World) return;

	const FAbilityContext Current = GetContext();

	FActorSpawnParameters Params;
	Params.Owner = Current.Instigator;
	Params.Instigator = Cast<APawn>(Current.Instigator);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedIndicator = World->SpawnActor<AActor>(Indicator, Current.Origin, Current.Direction.Rotation(), Params);

	if (!SpawnedIndicator || !Current.bFollowCaster || !IsValid(Current.Instigator)) return;

	SpawnedIndicator->AttachToActor(Current.Instigator, FAttachmentTransformRules::KeepWorldTransform);

	FTimerManagerTimerParameters EveryFrame;
	EveryFrame.bLoop = true;
	EveryFrame.bMaxOncePerFrame = true;

	FTimerHandle Timer;
	World->GetTimerManager().SetTimer(Timer, this, &UAbilityTask::UpdateIndicator, 0.001f, EveryFrame);
}

void UAbilityTask::UpdateIndicator() const
{
	if (IsValid(SpawnedIndicator)) SpawnedIndicator->SetActorRotation(GetContext().Direction.Rotation());
}
