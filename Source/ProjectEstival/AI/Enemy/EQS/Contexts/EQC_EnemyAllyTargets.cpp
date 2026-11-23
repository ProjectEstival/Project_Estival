// Fill out your copyright notice in the Description page of Project Settings.


#include "EQC_EnemyAllyTargets.h"

#include "EngineUtils.h"
#include "AI/Enemy/EnemyAIController.h"
#include "AI/Enemy/EnemyBaseCharacter.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEQC_EnemyAllyTargets::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner) return;
	
	UWorld* World = QueryOwner->GetWorld();
	if (!World) return;
	
	AEnemyBaseCharacter* QuerierEnemy = nullptr;
	if (AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(QueryOwner))
	{
		QuerierEnemy = Enemy;
	}
	else if (AEnemyAIController* Controller = Cast<AEnemyAIController>(QueryOwner))
	{
		QuerierEnemy = Cast<AEnemyBaseCharacter>(Controller->GetPawn());
	}
	
	TArray<AActor*> Allies;
	
	for (TActorIterator<AEnemyBaseCharacter> It(World); It; ++It)
	{
		AEnemyBaseCharacter* EnemyAlly = *It;
		
		if (EnemyAlly && EnemyAlly != QuerierEnemy)
		{
			Allies.Add(EnemyAlly);
		}
	}
	
	if (Allies.Num() > 0)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Allies);
	}
}
