// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSelfEffect.h"

TSet<AActor*> UTargetSelfEffect::GetTargets_Implementation(const FAbilityContext& Context) const
{
	return TSet<AActor*> { Context.Instigator };
}