#include "EnemyLookAtTargetTask.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/KismetMathLibrary.h"

EStateTreeRunStatus FEnemyLookAtTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FEnemyLookAtTargetTaskInstanceData& InstanceData = Context.GetInstanceData<FEnemyLookAtTargetTaskInstanceData>(*this);
	
	if (!IsValid(InstanceData.Player)) return EStateTreeRunStatus::Failed;
	
	FVector EnemyPos = InstanceData.Enemy->GetActorLocation();
	FVector PlayerPos = InstanceData.Player->GetActorLocation();
	
	// Turn Enemy
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(EnemyPos, PlayerPos);
	FRotator CurrentRot = InstanceData.Enemy->GetActorRotation();
	FRotator TargetRot(CurrentRot.Pitch, LookRot.Yaw, CurrentRot.Roll);
	
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, InstanceData.TurnSpeed);
	InstanceData.Enemy->SetActorRotation(NewRot);
	
	// Check if reached
	FVector EnemyForward = InstanceData.Enemy->GetActorForwardVector();
	FVector PlayerDist = (PlayerPos - EnemyPos).GetSafeNormal();
	float DotProd = FVector::DotProduct(EnemyForward, PlayerDist);
	
	if (DotProd >= InstanceData.MaxThreshold) return EStateTreeRunStatus::Succeeded;
	
	return EStateTreeRunStatus::Running;
}
