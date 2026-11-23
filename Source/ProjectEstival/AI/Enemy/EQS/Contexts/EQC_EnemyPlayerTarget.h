// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EQC_EnemyPlayerTarget.generated.h"

UCLASS()
class PROJECTESTIVAL_API UEQC_EnemyPlayerTarget : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
};
