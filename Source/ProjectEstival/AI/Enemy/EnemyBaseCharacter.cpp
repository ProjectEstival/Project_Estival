// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Enemy/EnemyBaseCharacter.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

void AEnemyBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (this->GetStatistics()->GetFloat(TagDefinitions::Stats_Resources_Health) <= 0.0f)
	{
		bIsDead = true;
	}
}
