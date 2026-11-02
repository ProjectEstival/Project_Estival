// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSelfEffect.h"

TArray<AActor*> UTargetSelfEffect::GetTargets_Implementation(const FAbilityContext& Context) const
{
	return TArray { Context.Instigator.Get() };
}