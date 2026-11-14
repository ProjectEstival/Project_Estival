// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Enemy/EnemyBaseCharacter.h"

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	this->OnDeath.AddDynamic(this, &AEnemyBaseCharacter::Dead);
}

void AEnemyBaseCharacter::Dead()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("DEAD"));
	
	this->Destroy();
}
