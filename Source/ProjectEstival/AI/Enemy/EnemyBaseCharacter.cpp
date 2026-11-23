// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Enemy/EnemyBaseCharacter.h"
#include "EnemyAIController.h"

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	this->OnDeath.AddDynamic(this, &AEnemyBaseCharacter::DeathAnim);
	
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &AEnemyBaseCharacter::HandleMontageNotifyBegin);
		}
	}
}

void AEnemyBaseCharacter::HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("DeathNotify"))
	{
		Dead();
	}
}

void AEnemyBaseCharacter::DeathAnim()
{
	if (!DeathMontage) return;
	
	this->PlayAnimMontage(DeathMontage);
}

void AEnemyBaseCharacter::Dead()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("DEAD"));
	
	this->Destroy();
}
