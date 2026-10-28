// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Stratos/SpawnerBase.h"
#include "SpawnerBase.h"
#include "EnemySpawner.h"


ASpawnerBase::ASpawnerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemySpawnerComponent =
		CreateDefaultSubobject<UEnemySpawner>(
			TEXT("EnemySpawnerComponent")
		);
}

void ASpawnerBase::TestSpawnEnemies()
{
	if (EnemySpawnerComponent)
	{
		EnemySpawnerComponent->SpawnEnemies();
	}
}


