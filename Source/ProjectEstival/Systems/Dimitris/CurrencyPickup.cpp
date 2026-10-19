#include "CurrencyPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"

ACurrencyPickup::ACurrencyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;

	CollisionSphere->InitSphereRadius(100.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
	CollisionSphere->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CollisionSphere);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACurrencyPickup::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(
	this, &ACurrencyPickup::OnPickupOverlap
);
}

void ACurrencyPickup::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);

	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (!GameInstance)
	{
		return;
	}

	UCurrencySubsystem* CurrencySystem =
		GameInstance->GetSubsystem<UCurrencySubsystem>();

	if (!CurrencySystem)
	{
		return;
	}

	const int32 PickupAmount = bRandomizeAmount
		? FMath::RandRange(
			FMath::Max(1, MinAmount),
			FMath::Max(FMath::Max(1, MinAmount), MaxAmount))
		: FMath::Max(1, Amount);

	bCollected = true;

	CurrencySystem->AddCurrency(CurrencyType, PickupAmount);

	UE_LOG(LogTemp, Log, TEXT("Collected %d currency of type %d"),
		PickupAmount, static_cast<int32>(CurrencyType));

	Destroy();
}

void ACurrencyPickup::SetCurrency(
	ECurrencyType Type,
	int32 NewAmount)
{
	CurrencyType = Type;
	Amount = FMath::Max(1, NewAmount);
	bRandomizeAmount = false;
}
