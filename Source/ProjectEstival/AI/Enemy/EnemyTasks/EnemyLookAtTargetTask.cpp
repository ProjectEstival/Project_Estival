#include "EnemyLookAtTargetTask.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus FEnemyLookAtTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FEnemyLookAtTargetTaskInstanceData& InstanceData = Context.GetInstanceData<FEnemyLookAtTargetTaskInstanceData>(*this);
	
	if (!IsValid(InstanceData.Player) || !IsValid(InstanceData.Enemy)) return EStateTreeRunStatus::Failed;
	
	FVector EnemyPos = InstanceData.Enemy->GetActorLocation();
	FVector PlayerPos = InstanceData.Player->GetActorLocation();
	
	// Turn Enemy
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(EnemyPos, PlayerPos);
	FRotator CurrentRot = InstanceData.Enemy->GetActorRotation();
	FRotator TargetRot(CurrentRot.Pitch, LookRot.Yaw, CurrentRot.Roll);
	
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, InstanceData.TurnSpeed);
	InstanceData.Enemy->SetActorRotation(NewRot);
	
	// Check if reached
		// Enemy Forward
	FVector EnemyForward = InstanceData.Enemy->GetActorForwardVector();
	EnemyForward.Z = 0.0f;
	EnemyForward.Normalize();
	
		// Player Dir
	FVector PlayerDir = PlayerPos - EnemyPos;
	PlayerDir.Z = 0.0f;
	PlayerDir.Normalize();
	
	float DotProd = FVector::DotProduct(EnemyForward, PlayerDir);
	
	if (DotProd >= InstanceData.MaxThreshold) return EStateTreeRunStatus::Succeeded;
	
	return EStateTreeRunStatus::Running;
}
