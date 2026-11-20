#include "VendorInteractables.h"
#include "Vendor.h"
#include "VendorData.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#pragma region AVendorInteractable
AVendorInteractable::AVendorInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(80.f);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(Trigger);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AVendorInteractable::CanInteract_Implementation(AActor* Interactor) const
{
	const AVendor* Vendor = GetVendor();
	return Vendor && Vendor->CanServe(Interactor);
}

void AVendorInteractable::OnReleased_Implementation()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
}

AVendor* AVendorInteractable::GetVendor() const
{
	return Cast<AVendor>(GetOwner());
}
#pragma endregion

#pragma region AVendorItem
AVendorItem::AVendorItem()
{
	// purchases are held; the reroll button keeps the base 0 (tap)
	HoldDuration = 0.5f;
}

void AVendorItem::BeginPlay()
{
	Super::BeginPlay();
	ApplyDefinition();
}

void AVendorItem::OnAcquired_Implementation()
{
	// the pool has just unhidden the actor; an empty slot must stay hidden
	ApplyDefinition();
}

void AVendorItem::OnReleased_Implementation()
{
	Super::OnReleased_Implementation();
	SetDefinition(nullptr);
}

bool AVendorItem::TryInteract_Implementation(AActor* Interactor)
{
	AVendor* Vendor = GetVendor();
	return Vendor && Vendor->TryPurchase(this, Interactor) == EVendorTransactionResult::Success;
}

void AVendorItem::SetDefinition(UVendorItemDefinition* NewDefinition)
{
	if (Definition == NewDefinition) return;

	Definition = NewDefinition;
	ApplyDefinition();
}

void AVendorItem::ApplyDefinition()
{
	const bool bStocked = Definition != nullptr;

	Mesh->SetStaticMesh(bStocked ? Definition->DisplayMesh.Get() : nullptr);
	SetActorHiddenInGame(!bStocked);
	Trigger->SetCollisionEnabled(bStocked ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	OnDefinitionChanged(Definition);
}
#pragma endregion

#pragma region AVendorRerollItem
AVendorRerollItem::AVendorRerollItem()
{
	// longer than a purchase, so players have time to change their mind and release
	HoldDuration = 1.5f;
}

bool AVendorRerollItem::TryInteract_Implementation(AActor* Interactor)
{
	AVendor* Vendor = GetVendor();
	return Vendor && Vendor->TryReroll(this, Interactor) == EVendorTransactionResult::Success;
}
#pragma endregion