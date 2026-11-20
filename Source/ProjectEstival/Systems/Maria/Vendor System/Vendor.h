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
	/** Nothing to buy: empty slot, reroll disabled, invalid request. */
	Unavailable,
	/** A grant refused (already maxed, already unlocked). The buyer was not charged. */
	NotGrantable,
	/** The buyer has no ICurrencyWallet. */
	NoWallet,
	NotEnoughCurrency
};

/** Fires after every purchase or reroll attempt, successful or not. Target is the item or button used. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVendorTransaction, AVendorInteractable*, Target, AActor*, Buyer, EVendorTransactionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVendorRestocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVendorInteracted, AActor*, Interactor);

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

	UFUNCTION(BlueprintPure, Category = "Vendor")
	bool CanReroll() const { return bAllowReroll; }

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

	/** Pool the stock is drawn from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<UVendorCatalog> Catalog;

	/** Spawned on every Product socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TSubclassOf<AVendorItem> ProductItemClass;

	/** Run vendors sell each item once. Permanent vendors turn this off, since their limit is the profile's purchase cap and potions must stay buyable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	bool bRemoveOnPurchase = true;

	/** Permanent vendors turn this off, which also skips the Reroll socket. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reroll")
	bool bAllowReroll = true;

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
	TArray<TObjectPtr<AVendorItem>> ProductItems;
#pragma endregion

#pragma region Private Methods
	void SpawnAtSocket(UVendorSocketComponent* Socket);
	void Restock();
	TArray<UVendorItemDefinition*> RollStock(int32 Count) const;

	EVendorTransactionResult ProcessPurchase(AVendorItem* Item, AActor* Buyer);
	EVendorTransactionResult ProcessReroll(AActor* Buyer);
	EVendorTransactionResult TryCharge(AActor* Buyer, const TArray<FVendorPrice>& Prices) const;
#pragma endregion
};