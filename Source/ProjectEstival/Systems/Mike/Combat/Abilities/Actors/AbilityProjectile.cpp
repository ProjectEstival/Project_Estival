#include "AbilityProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAbilityProjectile::AAbilityProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(20.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;
	
	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(Collision);
		Arrow->ArrowColor = FColor::Red;
	}
	
	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->InitialSpeed = 2000;
	Movement->MaxSpeed = 2000;
	Movement->ProjectileGravityScale = 0;
	Movement->bRotationFollowsVelocity = true;
}

void AAbilityProjectile::SetupProjectile(const float Speed, const float Range)
{
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	if (Speed > 0.f) SetLifetime(Range / Speed);

	Collision->ClearMoveIgnoreActors();
	if (AActor* Caster = GetAbilityContext().Instigator) Collision->IgnoreActorWhenMoving(Caster, true);
}

void AAbilityProjectile::StartAbilityActor()
{
	Movement->SetUpdatedComponent(Collision);
	Movement->Velocity = GetActorForwardVector() * Movement->InitialSpeed;

	Super::StartAbilityActor();

	TArray<AActor*> Touching;
	Collision->GetOverlappingActors(Touching);

	for (AActor* Actor : Touching)
	{
		if (CanHit(Actor)) TryHit(Actor);
	}
}

void AAbilityProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AAbilityProjectile::HandleOverlap);
	Movement->OnProjectileStop.AddDynamic(this, &AAbilityProjectile::HandleStop);
}

FAbilityContext AAbilityProjectile::MakeActorContext() const
{
	FAbilityContext ProjectileContext = Super::MakeActorContext();

	const FVector Travel = Movement->Velocity.GetSafeNormal2D();
	if (!Travel.IsNearlyZero()) ProjectileContext.Direction = Travel;

	return ProjectileContext;
}

bool AAbilityProjectile::CanHit(const AActor* Other) const
{
	return !IsExpired() && IsValid(Other) && Other != GetAbilityContext().Instigator.Get();
}

void AAbilityProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CanHit(OtherActor)) TryHit(OtherActor);
}

void AAbilityProjectile::HandleStop(const FHitResult& ImpactResult)
{
	Expire();
}
