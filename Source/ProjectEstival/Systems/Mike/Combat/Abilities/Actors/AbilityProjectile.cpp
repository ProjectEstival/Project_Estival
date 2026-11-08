#include "AbilityProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetingEffectBase.h"

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

void AAbilityProjectile::Initialize(const FAbilityContext& InContext, const UTargetingEffectBase* InEffect, const float Speed, const float Range)
{
	Context = InContext;
	Effect = InEffect;
	
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	if (Speed > 0.f) InitialLifeSpan = Range / Speed;
	
	if (AActor* Caster = Context.Instigator) Collision->IgnoreActorWhenMoving(Caster, true);
}

FGenericTeamId AAbilityProjectile::GetGenericTeamId() const
{
	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Context.Instigator.Get());
	return TeamAgent ? TeamAgent->GetGenericTeamId() : FGenericTeamId::NoTeam;
}

void AAbilityProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AAbilityProjectile::HandleOverlap);
	Movement->OnProjectileStop.AddDynamic(this, &AAbilityProjectile::HandleStop);
}

void AAbilityProjectile::Expire(const bool bHitWorld)
{
	if (bExpired) return;
	bExpired = true;
	
	OnExpired(bHitWorld);
	Destroy();
}

bool AAbilityProjectile::CanHit(AActor* Other) const
{
	if (bExpired || !IsValid(Other) || Other == Context.Instigator) return false;
	return true;
}

void AAbilityProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const UTargetingEffectBase* Source = Effect.Get();
	if (!Source || !CanHit(OtherActor)) return;
	
	FAbilityContext HitContext = Context;
	HitContext.Origin = GetActorLocation();
	HitContext.Direction = Movement->Velocity.GetSafeNormal2D();
	
	if (!Source->TryApplyOnTarget(HitContext, OtherActor, Hits)) return;
	
	OnHitActor(OtherActor);
	if (Source->IsExhausted(Hits)) Expire(false);
}

void AAbilityProjectile::LifeSpanExpired()
{
	Expire(false);
}

void AAbilityProjectile::HandleStop(const FHitResult& ImpactResult)
{
	Expire(true);
}