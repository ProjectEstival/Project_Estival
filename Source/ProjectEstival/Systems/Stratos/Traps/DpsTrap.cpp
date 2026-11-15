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



// Called every frame
void ADpsTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

