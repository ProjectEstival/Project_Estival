// Fill out your copyright notice in the Description page of Project Settings.


#include "AttitudeFilterPayload.h"
#include "GenericTeamAgentInterface.h"

UAttitudeFilterPayload::UAttitudeFilterPayload()
{
	SuccessRequirement = ChainPartial;
}

bool UAttitudeFilterPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Context.Instigator) || !IsValid(Target)) return false;
	const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(Context.Instigator, Target);
	return AllowedAttitudes.Contains(Attitude);
}