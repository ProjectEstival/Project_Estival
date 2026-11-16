// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Stratos/Traps/DpsTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
ADpsTrap::ADpsTrap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	TriggerArea = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerArea"));
	RootComponent = TriggerArea;
	TriggerArea->SetCollisionProfileName(TEXT("Trigger"));
	
	
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetVisibility(false);
	
	
	CurrentState = ETrapState::Idle;
}

// Called when the game starts or when spawned
void ADpsTrap::BeginPlay()
{
	Super::BeginPlay();
	
	
	TriggerArea->OnComponentBeginOverlap.AddDynamic(this, &ADpsTrap::OnOverlapBegin);
	TriggerArea->OnComponentEndOverlap.AddDynamic(this, &ADpsTrap::OnOverlapEnd);
}

void ADpsTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		ActorsInArea.AddUnique(OtherActor);

		if (CurrentState == ETrapState::Idle)
		{
			SetState(ETrapState::Telegraph);
		}
	}
}

void ADpsTrap::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ActorsInArea.Remove(OtherActor);
	}
}


void ADpsTrap::SetState(ETrapState NewState)
{
    CurrentState = NewState;

    switch (CurrentState)
    {
        case ETrapState::Telegraph:
            UE_LOG(LogTemp, Warning, TEXT("Trap: Telegraph (%f sec)"), TelegraphDelay);
            GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &ADpsTrap::ActivateTrap, TelegraphDelay, false);
            break;

        case ETrapState::Active:
            UE_LOG(LogTemp, Warning, TEXT("Trap: ACTIVE! Poison Cloud On."));
            VisualMesh->SetVisibility(true);
            
            GetWorld()->GetTimerManager().SetTimer(DpsTimerHandle, this, &ADpsTrap::ApplyDpsTick, DpsRate, true);
            
            GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &ADpsTrap::EndTrapActive, Active, false);
            break;

        case ETrapState::Cooldown:
            UE_LOG(LogTemp, Warning, TEXT("Trap: Cooldown."));
            VisualMesh->SetVisibility(false);
            GetWorld()->GetTimerManager().ClearTimer(DpsTimerHandle);
            
            GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &ADpsTrap::ResetToIdle, Cooldown, false);
            break;

        case ETrapState::Idle:
            UE_LOG(LogTemp, Warning, TEXT("Trap: Idle."));
            
            if (ActorsInArea.Num() > 0)
            {
                SetState(ETrapState::Telegraph);
            }
            break;
    }
}

void ADpsTrap::ActivateTrap() { SetState(ETrapState::Active); }
void ADpsTrap::EndTrapActive() { SetState(ETrapState::Cooldown); }
void ADpsTrap::ResetToIdle() { SetState(ETrapState::Idle); }

void ADpsTrap::ApplyDpsTick()
{
    for (AActor* ActorInZone : ActorsInArea)
    {
        if (IsValid(ActorInZone))
        {
            UE_LOG(LogTemp, Error, TEXT("Trap dealt %f damage to %s!"), DamagePerRate, *ActorInZone->GetName());
            UGameplayStatics::ApplyDamage(
                ActorInZone,
                DamagePerRate,
                nullptr,
                this,
                UDamageType::StaticClass()
            );
        }
    }
}


// Called every frame
void ADpsTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

