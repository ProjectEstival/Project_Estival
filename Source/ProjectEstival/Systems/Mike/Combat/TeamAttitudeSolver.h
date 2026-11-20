#pragma once

#include "GenericTeamAgentInterface.h"

namespace RunAttitudes
{
	inline ETeamAttitude::Type GetTeamBasedAttitude(const FGenericTeamId TeamA, const FGenericTeamId TeamB)
	{
		if (TeamA == FGenericTeamId::NoTeam || TeamB == FGenericTeamId::NoTeam)
		{
			return ETeamAttitude::Neutral;
		}
	
		if (TeamA == TeamB) return ETeamAttitude::Friendly;
		return ETeamAttitude::Hostile;
	}
	inline ETeamAttitude::Type GetFreeForAllAttitude(const FGenericTeamId TeamA, const FGenericTeamId TeamB)
	{
		if (TeamA == FGenericTeamId::NoTeam || TeamB == FGenericTeamId::NoTeam)
		{
			return ETeamAttitude::Neutral;
		}
		
		return ETeamAttitude::Hostile;
	}
}