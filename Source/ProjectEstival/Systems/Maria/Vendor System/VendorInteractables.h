// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/Maria/Interaction System/Interactable.h"
#include "Systems/Maria/Pooling System/Poolable.h"
#include "VendorInteractables.generated.h"

class AVendor;
class USphereComponent;
class UStaticMeshComponent;
class UVendorItemDefinition;

/** Base of everything the player uses at a vendor. Acquired from the actor pool by AVendor, which owns all the logic. */
UCLASS(Abstract)
class PROJECTESTIVAL_API AVendorInteractable : public AActor, public IInteractable, public IPoolable
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	AVendorInteractable();

	/** Delegates to the vendor, which decides whether it is serving this interactor. Items and the reroll button have no gate of their own. */
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual float GetHoldDuration_Implementation() const override { return HoldDuration; }

	/** Detaches from the vendor so the pool can reuse the actor. */
	virtual void OnReleased_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Vendor")
	AVendor* GetVendor() const;
#pragma endregion

protected:
#pragma region Serialized Fields
	/** Overlaps every channel so the interaction query finds it without blocking the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Seconds the player must hold the input. 0 = instant. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0", Units = "s"))
	float HoldDuration = 0.f;
#pragma endregion
};

/** One product on display. Empty (no definition) once sold or when the catalog ran out. */
UCLASS()
class PROJECTESTIVAL_API AVendorItem : public AVendorInteractable
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	AVendorItem();

	/** Affordability is checked here, on completion of the hold, so the player gets failure feedback through the vendor's OnTransaction. */
	virtual bool TryInteract_Implementation(AActor* Interactor) override;

	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	/** Null empties the slot. */
	void SetDefinition(UVendorItemDefinition* NewDefinition);

	UFUNCTION(BlueprintPure, Category = "Vendor")
	UVendorItemDefinition* GetDefinition() const { return Definition; }
#pragma endregion

protected:
	virtual void BeginPlay() override;

	/** Presentation hook (name, icon, price widgets). Runs on every machine when the stock changes. Null = empty slot. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor")
	void OnDefinitionChanged(UVendorItemDefinition* NewDefinition);

private:
#pragma region Private Fields
	UPROPERTY()
	TObjectPtr<UVendorItemDefinition> Definition;
#pragma endregion

#pragma region Private Methods
	void ApplyDefinition();
#pragma endregion
};

/** The reroll button. Only spawned when the vendor allows rerolls. */
UCLASS()
class PROJECTESTIVAL_API AVendorRerollItem : public AVendorInteractable
{
	GENERATED_BODY()

public:
	AVendorRerollItem();

	/** Also false once the vendor's rerolls are used up, so the button can't be started and a hold in progress is cancelled. */
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool TryInteract_Implementation(AActor* Interactor) override;
};