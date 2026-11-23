#pragma once

#include "Systems/Maria/Interaction System/InteractionComponent.h"
#include "Systems/Maria/Revive System/ReviveComponent.h"
#include "CharacterBase.h"
#include "CoreMinimal.h"
#include "InputAction.h"

#include "PlayerBase.generated.h"

class UMaterialInstanceDynamic;
class UDecalComponent;

UCLASS()
class PROJECTESTIVAL_API APlayerBase : public AEstivalBaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerBase();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void SetPlayerColor(const FLinearColor& Color);
	
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resurrection")
	TObjectPtr<UReviveComponent> ResurrectionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UDecalComponent> ColorRing;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ColorRingMaterial = nullptr;
	
	void StartInteract();
	void StopInteract();
};
