#include "DungeonGameMode.h"
#include "GenericTeamAgentInterface.h"
#include "Systems/Mike/Combat/TeamAttitudeSolver.h"

void ADungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	FGenericTeamId::SetAttitudeSolver(&GetTeamBasedAttitude);
}