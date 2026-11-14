// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "EstivalGameInstance.generated.h"

UCLASS()
class PROJECTESTIVAL_API UEstivalGameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta = (MustImplement = "/Script/ProjectEstival.Poolable"))
	TMap<TSubclassOf<AActor>, FActorPoolSettings> ActorPoolSettings;
	
public:
	FActorPoolSettings GetActorPoolSettings(const TSubclassOf<AActor> ActorClass) const
	{
		return ActorPoolSettings.FindRef(ActorClass);
	}
};
