// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Public/Characters/Components/AC_ActionComponent.h"
#include "AC_AbilityComponent.generated.h"

/**
 *  Casts a spell :)
 */
UCLASS(ClassGroup=(Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UAC_AbilityComponent : public UAC_ActionComponent
{
	GENERATED_BODY()

public:

	/** Actor spawned when the ability is cast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	TSubclassOf<AActor> SpawnClass;

	/** Distance in front of the owner to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability", meta = (Units = "cm"))
	float SpawnOffset = 100.0f;

	/** Ammount of uses, -1 = infinite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability", meta = (ClampMin = -1))
	int32 Charges = -1;

	virtual bool CanActivate() const override;

protected:

	virtual void Execute() override;
};