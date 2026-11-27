

#include "Vendor.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "Systems/Dimitris/CurrencySubsystem.h"
#include "VendorProfileAccess.h"
#include "VendorSocketComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace
{
	/** Picks one entry. A tiered item's chance is its tier's SpawnWeight shared among that tier's candidates by their own weights,
	 * so a tier turns up as often as its weight says however many items it holds. An item without a tier uses its weight as is.
	 * */
	UVendorItemDefinition* DrawWeighted(const TArray<const FVendorCatalogEntry*>& Pool)
	{
		TMap<const UVendorTier*, float> TierTotals;
		for (const FVendorCatalogEntry* Entry : Pool)
		{
			if (const UVendorTier* Tier = Entry->Item->Tier) TierTotals.FindOrAdd(Tier) += Entry->Weight;
		}

		TArray<float> Chances;
		float TotalChance = 0.f;
		for (const FVendorCatalogEntry* Entry : Pool)
		{
			const UVendorTier* Tier = Entry->Item->Tier;
			const float Chance = Tier ? Entry->Weight / TierTotals.FindChecked(Tier) * Tier->SpawnWeight : Entry->Weight;
			Chances.Add(Chance);
			TotalChance += Chance;
		}
		if (TotalChance <= 0.f) return nullptr;

		float Roll = FMath::FRandRange(0.f, TotalChance);
		int32 Picked = INDEX_NONE;
		for (int32 i = 0; i < Pool.Num(); ++i)
		{
			if (Chances[i] <= 0.f) continue;

			// the last entry that can be drawn is the fallback for float rounding
			Picked = i;
			Roll -= Chances[i];
			if (Roll < 0.f) break;
		}
		return Pool[Picked]->Item.Get();
	}
}

#pragma region Public Methods
AVendor::AVendor()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(150.f);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(Trigger);
}

bool AVendor::CanInteract_Implementation(AActor* Interactor) const { return CanServe(Interactor); }

bool AVendor::TryInteract_Implementation(AActor* Interactor)
{
	OnInteracted.Broadcast(Interactor);
	return true;
}

void AVendor::BeginPlay()
{
	Super::BeginPlay();

	TArray<UVendorSocketComponent*> Sockets;
	GetComponents(Sockets);
	for (UVendorSocketComponent* Socket : Sockets) SpawnAtSocket(Socket);

	Restock(false);
}

void AVendor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// a world teardown takes the pool with it, so only a vendor leaving a living world returns its items
	const bool bWorldContinues = EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld;
	if (bWorldContinues && GetWorld())
	{
		if (UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
		{
			for (AVendorInteractable* Spawned : SpawnedInteractables) Pool->ReleaseActor(Spawned);
		}
		SpawnedInteractables.Reset();
		Slots.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

EVendorTransactionResult AVendor::TryPurchase(AVendorItem* Item, AActor* Buyer)
{
	const EVendorTransactionResult Result = ProcessPurchase(Item, Buyer);
	OnTransaction.Broadcast(Item, Buyer, Result);
	return Result;
}

EVendorTransactionResult AVendor::TryReroll(AVendorRerollItem* Button, AActor* Buyer)
{
	const EVendorTransactionResult Result = ProcessReroll(Buyer);
	OnTransaction.Broadcast(Button, Buyer, Result);
	return Result;
}
#pragma endregion

#pragma region Private Methods
void AVendor::SpawnAtSocket(UVendorSocketComponent* Socket)
{
	const bool bProduct = Socket->Type == EVendorSocketType::Product;
	if (!bProduct && !bAllowReroll) return;

	UClass* ItemClass = bProduct ? ProductItemClass.Get() : RerollItemClass.Get();
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no item class set for socket '%s'."), *GetNameSafe(this), *GetNameSafe(Socket));
		return;
	}

	UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	AVendorInteractable* Spawned = Pool ? Cast<AVendorInteractable>(Pool->AcquireActor(ItemClass, Socket->GetComponentTransform())) : nullptr;
	if (!Spawned) return;

	// after the acquire, because OnReleased clears the owner and attachment
	Spawned->SetOwner(this);
	Spawned->AttachToComponent(Socket, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedInteractables.Add(Spawned);
	AVendorItem* Item = Cast<AVendorItem>(Spawned);
	if (!Item) return;

	FVendorSlot& Slot = Slots.AddDefaulted_GetRef();
	Slot.Item = Item;
	Slot.Catalog = Socket->CatalogOverride ? Socket->CatalogOverride.Get() : Catalog.Get();
	Slot.bRerollable = Socket->bRerollable;
}

void AVendor::Restock(const bool bRerollOnly)
{
	TSet<const UVendorItemDefinition*> Avoid = GetDisplayedDefinitions();

	for (const FVendorSlot& Slot : Slots)
	{
		if (bRerollOnly && !Slot.bRerollable) continue;

		UVendorItemDefinition* Picked = RollItem(Slot.Catalog, Avoid);
		Slot.Item->SetDefinition(Picked);
		if (Picked) Avoid.Add(Picked);
	}

	OnRestocked.Broadcast();
}

void AVendor::RefillSlot(FVendorSlot& Slot)
{
	// the item just sold is still on display here, so the new roll avoids it
	Slot.Item->SetDefinition(RollItem(Slot.Catalog, GetDisplayedDefinitions()));
}

TSet<const UVendorItemDefinition*> AVendor::GetDisplayedDefinitions() const
{
	TSet<const UVendorItemDefinition*> Displayed;
	for (const FVendorSlot& Slot : Slots)
	{
		if (Slot.Item && Slot.Item->GetDefinition()) Displayed.Add(Slot.Item->GetDefinition());
	}
	return Displayed;
}

UVendorItemDefinition* AVendor::RollItem(const UVendorCatalog* SourceCatalog, const TSet<const UVendorItemDefinition*>& Avoid) const
{
	if (!SourceCatalog) return nullptr;

	TArray<const FVendorCatalogEntry*> Fresh;
	TArray<const FVendorCatalogEntry*> All;
	for (const FVendorCatalogEntry& Entry : SourceCatalog->Entries)
	{
		if (!Entry.Item || Entry.Weight <= 0.f) continue;

		All.Add(&Entry);
		if (!Avoid.Contains(Entry.Item.Get())) Fresh.Add(&Entry);
	}
	return DrawWeighted(Fresh.IsEmpty() ? All : Fresh);
}

EVendorTransactionResult AVendor::ProcessPurchase(AVendorItem* Item, AActor* Buyer)
{
	FVendorSlot* Slot = Slots.FindByPredicate([Item](const FVendorSlot& Entry) { return Entry.Item == Item; });
	UVendorItemDefinition* Definition = Slot ? Item->GetDefinition() : nullptr;
	if (!Definition || !Buyer) return EVendorTransactionResult::Unavailable;

	for (const UVendorGrant* Reward : Definition->Grants) { if (Reward && !Reward->CanGrant(Buyer)) return EVendorTransactionResult::NotGrantable; }

	const EVendorTransactionResult Payment = TryCharge(Buyer, Definition->GetPrices());
	if (Payment != EVendorTransactionResult::Success) return Payment;

	for (const UVendorGrant* Reward : Definition->Grants) { if (Reward) Reward->Grant(Buyer); }

	if (bRefillOnPurchase) RefillSlot(*Slot);
	return EVendorTransactionResult::Success;
}

EVendorTransactionResult AVendor::ProcessReroll(AActor* Buyer)
{
	if (!Buyer || !CanReroll()) return EVendorTransactionResult::Unavailable;

	const EVendorTransactionResult Payment = TryCharge(Buyer, RerollPrices);
	if (Payment != EVendorTransactionResult::Success) return Payment;

	RerollsUsed++;
	Restock(true);
	return EVendorTransactionResult::Success;
}

EVendorTransactionResult AVendor::TryCharge(AActor* Buyer, const TArray<FVendorPrice>& Prices) const
{
	// prices on the same balance are summed, so two entries can't each pass the check and overdraw together
	TMap<ECurrencyStorage, int32> Totals;
	for (const FVendorPrice& Price : Prices)
	{
		if (Price.Amount > 0) Totals.FindOrAdd(Price.Storage) += Price.Amount;
	}
	if (Totals.IsEmpty()) return EVendorTransactionResult::Success;

	ULocalPlayer* LocalPlayer = FVendorProfileAccess::FindLocalPlayer(Buyer);
	UCurrencySubsystem* Currency = UGameInstance::GetSubsystem<UCurrencySubsystem>(GetGameInstance());
	if (!LocalPlayer || !Currency) return EVendorTransactionResult::NoPlayer;

	for (const TPair<ECurrencyStorage, int32>& Total : Totals)
	{
		if (Currency->GetCurrency(LocalPlayer, Total.Key) < Total.Value) return EVendorTransactionResult::NotEnoughCurrency;
	}

	TArray<TPair<ECurrencyStorage, int32>> Charged;
	for (const TPair<ECurrencyStorage, int32>& Total : Totals)
	{
		if (!Currency->TryConsumeCurrency(LocalPlayer, Total.Key, Total.Value))
		{
			// the permanent balance can fail to save, so what was already taken is given back and a multi-currency price never charges half
			for (const TPair<ECurrencyStorage, int32>& Taken : Charged) Currency->AddCurrency(LocalPlayer, Taken.Key, Taken.Value);
			return EVendorTransactionResult::NotEnoughCurrency;
		}
		Charged.Emplace(Total.Key, Total.Value);
	}
	return EVendorTransactionResult::Success;
}
#pragma endregion