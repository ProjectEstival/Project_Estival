#include "Vendor.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "CurrencyWallet.h"
#include "VendorSocketComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"

namespace
{
	UObject* FindWallet(AActor* Buyer)
	{
		if (Buyer->Implements<UCurrencyWallet>()) return Buyer;

		const TArray<UActorComponent*> Wallets = Buyer->GetComponentsByInterface(UCurrencyWallet::StaticClass());
		return Wallets.Num() > 0 ? Wallets[0] : nullptr;
	}

	/** Picks one entry by weight and removes it from the pool so a roll never repeats an item. */
	UVendorItemDefinition* DrawWeighted(TArray<const FVendorCatalogEntry*>& Pool)
	{
		float TotalWeight = 0.f;
		for (const FVendorCatalogEntry* Entry : Pool) TotalWeight += Entry->Weight;

		float Roll = FMath::FRandRange(0.f, TotalWeight);
		// last entry is the fallback for float rounding
		int32 Index = Pool.Num() - 1;
		for (int32 i = 0; i < Pool.Num(); ++i)
		{
			Roll -= Pool[i]->Weight;
			if (Roll > 0.f) continue;
			Index = i;
			break;
		}

		UVendorItemDefinition* Picked = Pool[Index]->Item.Get();
		Pool.RemoveAtSwap(Index);
		return Picked;
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

bool AVendor::CanInteract_Implementation(AActor* Interactor) const
{
	return CanServe(Interactor);
}

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

	Restock();
}

void AVendor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// a world teardown takes the pool with it, so only a vendor leaving a living world returns its items
	const bool bWorldContinues = EndPlayReason == EEndPlayReason::Destroyed || 
		EndPlayReason == EEndPlayReason::RemovedFromWorld;
	if (bWorldContinues && GetWorld())
	{
		if (UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
		{
			for (AVendorInteractable* Spawned : SpawnedInteractables) Pool->ReleaseActor(Spawned);
		}
		SpawnedInteractables.Reset();
		ProductItems.Reset();
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
		UE_LOG(LogTemp, Warning, TEXT("%s: no item class set for socket '%s'."), *GetNameSafe(this),
			*GetNameSafe(Socket));
		return;
	}

	UActorPoolSubsystem* Pool = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	AVendorInteractable* Spawned = Pool ? Cast<AVendorInteractable>(Pool->AcquireActor
		(ItemClass, Socket->GetComponentTransform())) : nullptr;
	if (!Spawned) return;

	// after the acquire, because OnReleased clears the owner and attachment
	Spawned->SetOwner(this);
	Spawned->AttachToComponent(Socket, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedInteractables.Add(Spawned);
	if (AVendorItem* Item = Cast<AVendorItem>(Spawned)) ProductItems.Add(Item);
}

void AVendor::Restock()
{
	const TArray<UVendorItemDefinition*> Picks = RollStock(ProductItems.Num());

	for (int32 i = 0; i < ProductItems.Num(); ++i)
	{
		ProductItems[i]->SetDefinition(Picks.IsValidIndex(i) ? Picks[i] : nullptr);
	}

	OnRestocked.Broadcast();
}

TArray<UVendorItemDefinition*> AVendor::RollStock(const int32 Count) const
{
	TArray<UVendorItemDefinition*> Result;
	if (!Catalog) return Result;

	TSet<const UVendorItemDefinition*> Current;
	for (const AVendorItem* Item : ProductItems)
	{
		if (Item && Item->GetDefinition()) Current.Add(Item->GetDefinition());
	}

	// a reroll prefers items not on display, and only falls back to them when the catalog is too small
	TArray<const FVendorCatalogEntry*> Fresh;
	TArray<const FVendorCatalogEntry*> Previous;
	for (const FVendorCatalogEntry& Entry : Catalog->Entries)
	{
		if (!Entry.Item || Entry.Weight <= 0.f) continue;
		(Current.Contains(Entry.Item.Get()) ? Previous : Fresh).Add(&Entry);
	}

	while (Result.Num() < Count && (Fresh.Num() > 0 || Previous.Num() > 0))
	{
		TArray<const FVendorCatalogEntry*>& Pool = Fresh.Num() > 0 ? Fresh : Previous;
		Result.Add(DrawWeighted(Pool));
	}
	return Result;
}

EVendorTransactionResult AVendor::ProcessPurchase(AVendorItem* Item, AActor* Buyer)
{
	UVendorItemDefinition* Definition = Item ? Item->GetDefinition() : nullptr;
	if (!Definition || !Buyer || !ProductItems.Contains(Item)) return EVendorTransactionResult::Unavailable;

	for (const UVendorGrant* Reward : Definition->Grants)
	{
		if (Reward && !Reward->CanGrant(Buyer)) return EVendorTransactionResult::NotGrantable;
	}

	const EVendorTransactionResult Payment = TryCharge(Buyer, Definition->Prices);
	if (Payment != EVendorTransactionResult::Success) return Payment;

	for (const UVendorGrant* Reward : Definition->Grants)
	{
		if (Reward) Reward->Grant(Buyer);
	}

	Item->SetDefinition(nullptr);
	return EVendorTransactionResult::Success;
}

EVendorTransactionResult AVendor::ProcessReroll(AActor* Buyer)
{
	if (!bAllowReroll || !Buyer) return EVendorTransactionResult::Unavailable;

	const EVendorTransactionResult Payment = TryCharge(Buyer, RerollPrices);
	if (Payment != EVendorTransactionResult::Success) return Payment;

	Restock();
	return EVendorTransactionResult::Success;
}

EVendorTransactionResult AVendor::TryCharge(AActor* Buyer, const TArray<FVendorPrice>& Prices) const
{
	const bool bFree = !Prices.ContainsByPredicate([](const FVendorPrice& Price) { return Price.Amount > 0; });
	if (bFree) return EVendorTransactionResult::Success;

	UObject* Wallet = FindWallet(Buyer);
	if (!Wallet) return EVendorTransactionResult::NoWallet;

	// check everything first so a multi-currency price never charges half
	for (const FVendorPrice& Price : Prices)
	{
		if (Price.Amount > 0 && !ICurrencyWallet::Execute_CanAfford(Wallet, Price.Currency, Price.Amount))
			return EVendorTransactionResult::NotEnoughCurrency;
	}

	for (const FVendorPrice& Price : Prices)
	{
		if (Price.Amount > 0 && !ICurrencyWallet::Execute_TrySpend(Wallet, Price.Currency, Price.Amount))
			return EVendorTransactionResult::NotEnoughCurrency;
	}
	return EVendorTransactionResult::Success;
}
#pragma endregion