// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Enemy/EyeballQueryContext.h"

#include "EnemyAIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Systems/Core/Characters/CharacterBase.h"

void UEyeballQueryContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner) return;
	
	AEnemyAIController* Controller = nullptr;
	
	if (APawn* Enemy = Cast<APawn>(QueryOwner))
	{
		Controller = Cast<AEnemyAIController>(Enemy->GetController());
	}
	else
	{
		Controller = Cast<AEnemyAIController>(QueryOwner);
	}
	
	if (Controller && IsValid(Controller->PlayerRef))
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Controller->PlayerRef);
	}
}
