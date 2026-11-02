#pragma once

#include "GenericTeamAgentInterface.h"

inline ETeamAttitude::Type GetTeamBasedAttitude(const FGenericTeamId TeamA, const FGenericTeamId TeamB)
{
	if (TeamA == FGenericTeamId::NoTeam || TeamB == FGenericTeamId::NoTeam)
	{
		return ETeamAttitude::Neutral;
	}
	
	if (TeamA == TeamB) return ETeamAttitude::Friendly;
	return ETeamAttitude::Hostile;
}