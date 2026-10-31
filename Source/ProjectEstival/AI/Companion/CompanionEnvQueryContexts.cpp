#include "AI/Companion/CompanionEnvQueryContexts.h"
#include "AI/Companion/CompanionAIController.h"
#include "EngineUtils.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Navigation/PathFollowingComponent.h"

void UEnvQueryContext_CompanionFollowTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// the querier is whoever run the query
	const APawn* Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	const ACompanionAIController* Companion = Pawn ? Cast<ACompanionAIController>(Pawn->GetController()) : nullptr;

	if (Companion && Companion->GetFollowTarget())
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Companion->GetFollowTarget());
	}
}

void UEnvQueryContext_OtherCompanions::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const APawn* MyPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!MyPawn)
	{
		return;
	}

	TArray<FVector> Spots;
	for (TActorIterator<ACompanionAIController> It(MyPawn->GetWorld()); It; ++It)
	{
		const ACompanionAIController* Other = *It;
		const APawn* OtherPawn = Other->GetPawn();
		if (!OtherPawn || OtherPawn == MyPawn)
		{
			continue;
		}

		// a walking companion has already "claimed" its destination
		if (Other->GetMoveStatus() == EPathFollowingStatus::Moving)
		{
			Spots.Add(Other->GetPathFollowingComponent()->GetPathDestination());
		}
		else
		{
			Spots.Add(OtherPawn->GetActorLocation());
		}
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Spots);
}
