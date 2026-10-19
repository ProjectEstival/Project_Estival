// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSelf.h"

TSet<AActor*> UTargetSelf::GetTargets_Implementation(const FAbilityContext& Context) const
{
	return TSet<AActor*> { Context.Instigator };
}