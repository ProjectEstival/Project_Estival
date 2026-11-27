// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/Maria/Interaction System/Interactable.h"
#include "VendorData.h"
#include "VendorInteractables.h"
#include "Vendor.generated.h"

class USphereComponent;
class UVendorSocketComponent;

UENUM(BlueprintType)
enum class EVendorTransactionResult : uint8
{
	Success,
	/** Nothing to buy: empty slot, reroll disabled or used up, invalid request. */
	Unavailable,
	/** A grant refused (already maxed, already unlocked). The buyer was not charged. */
	NotGrantable,
	/** The buyer isn't a local player with a profile, so there is no balance to charge. */
	NoPlayer,
	NotEnoughCurrency
};

/** Fires after every purchase or reroll attempt, successful or not. Target is the item or button used. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVendorTransaction, AVendorInteractable*, Target, AActor*, Buyer, EVendorTransactionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVendorRestocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVendorInteracted, AActor*, Interactor);

/** One product slot: the item on display and where its stock comes from. */
USTRUCT()
struct FVendorSlot
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AVendorItem> Item;

	UPROPERTY()
	TObjectPtr<UVendorCatalog> Catalog;

	UPROPERTY()
	bool bRerollable = true;
};

/** Sells items from a catalog at the sockets placed in its Blueprint (table, NPC, props).
 * Run vendors and permanent vendors are the same class with different catalogs, currencies and reroll setting.
 * */
UCLASS()
class PROJECTESTIVAL_API AVendor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	AVendor();

	/** Talking to the vendor himself: a tap, gated by CanServe. */
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual float GetHoldDuration_Implementation() const override { return 0.f; }
	virtual bool TryInteract_Implementation(AActor* Interactor) override;

	/** Whether the vendor is serving this interactor right now (open, buyer alive...). Override in the Blueprint to close the shop. */
	UFUNCTION(BlueprintNativeEvent, Category = "Vendor")
	bool CanServe(AActor* Buyer) const;
	virtual bool CanServe_Implementation(AActor* Buyer) const { return true; }

	/** Charges the buyer and grants the item. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	EVendorTransactionResult TryPurchase(AVendorItem* Item, AActor* Buyer);

	/** Charges the buyer and replaces all stock with a fresh roll. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	EVendorTransactionResult TryReroll(AVendorRerollItem* Button, AActor* Buyer);

	/** False when rerolls are disabled or all of them were used. A reroll button should lock or hide itself on this after OnRestocked. */
	UFUNCTION(BlueprintPure, Category = "Vendor")
	bool CanReroll() const { return bAllowReroll && (MaxRerolls == 0 || RerollsUsed < MaxRerolls); }

	UFUNCTION(BlueprintPure, Category = "Vendor")
	TArray<FVendorPrice> GetRerollPrices() const { return RerollPrices; }
#pragma endregion

#pragma region Events
	UPROPERTY(BlueprintAssignable) FOnVendorTransaction OnTransaction;

	/** Stock was rolled, at start and on every reroll. */
	UPROPERTY(BlueprintAssignable) FOnVendorRestocked OnRestocked;

	/** The player tapped the vendor. Hook dialogue, greeting or animation here. */
	UPROPERTY(BlueprintAssignable) FOnVendorInteracted OnInteracted;
#pragma endregion

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Serialized Fields
	/** Makes the vendor findable by the interaction query. Overlaps every channel so it never blocks the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<USphereComponent> Trigger;

	/** Pool the stock is drawn from. A socket can override it for its own slot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<UVendorCatalog> Catalog;

	/** Spawned on every Product socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TSubclassOf<AVendorItem> ProductItemClass;

	/** A bought item is replaced by a new roll. Permanent vendors turn this off, so the item stays on sale and their limit is the profile's purchase cap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	bool bRefillOnPurchase = true;

	/** Permanent vendors turn this off, which also skips the Reroll socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reroll")
	bool bAllowReroll = true;

	/** How many times the stock can be rerolled, by anyone, before the button locks. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reroll", meta = (ClampMin = "0", EditCondition = "bAllowReroll", EditConditionHides))
	int32 MaxRerolls = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reroll", meta = (EditCondition = "bAllowReroll", EditConditionHides))
	TSubclassOf<AVendorRerollItem> RerollItemClass;

	/** One entry per currency. Empty = free. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reroll", meta = (EditCondition = "bAllowReroll", EditConditionHides))
	TArray<FVendorPrice> RerollPrices;
#pragma endregion

private:
#pragma region Private Fields
	/** Everything acquired from the pool, returned on EndPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AVendorInteractable>> SpawnedInteractables;

	UPROPERTY(Transient)
	TArray<FVendorSlot> Slots;

	/** Kept on the vendor, so every player shares one limit and it can't be dodged by leaving and coming back. */
	UPROPERTY(Transient)
	int32 RerollsUsed = 0;
#pragma endregion

#pragma region Private Methods
	void SpawnAtSocket(UVendorSocketComponent* Socket);
	/** Rolls every slot, or only the rerollable ones. */
	void Restock(bool bRerollOnly);
	void RefillSlot(FVendorSlot& Slot);
	TSet<const UVendorItemDefinition*> GetDisplayedDefinitions() const;

	/** A roll prefers items that aren't in Avoid (on display or already picked) and only falls back to them when the catalog has nothing else. */
	UVendorItemDefinition* RollItem(const UVendorCatalog* SourceCatalog, const TSet<const UVendorItemDefinition*>& Avoid) const;

	EVendorTransactionResult ProcessPurchase(AVendorItem* Item, AActor* Buyer);
	EVendorTransactionResult ProcessReroll(AActor* Buyer);
	EVendorTransactionResult TryCharge(AActor* Buyer, const TArray<FVendorPrice>& Prices) const;
#pragma endregion
};