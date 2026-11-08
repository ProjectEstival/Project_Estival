#include "AI/Companion/CompanionEnvQueryTargeting.h"
#include "AI/Companion/CompanionAIController.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/TagDefinitions.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "AI/Enemy/EnemyAIController.h"

// ---------- Companion Threats ----------
UEnvQueryGenerator_CompanionThreats::UEnvQueryGenerator_CompanionThreats(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// this generator makes actors not points!!
	ItemType = UEnvQueryItemType_Actor::StaticClass();
}

void UEnvQueryGenerator_CompanionThreats::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	// the querier is the bot's pawn: Find Threat asks with the pawn
	const APawn* Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	const ACompanionAIController* Companion = Pawn ? Cast<ACompanionAIController>(Pawn->GetController()) : nullptr;
	if (!Companion)
	{
		return;
	}

	for (AActor* Threat : Companion->GetThreats())
	{
		QueryInstance.AddItemData<UEnvQueryItemType_Actor>(Threat);
	}
}

// ---------- Companion: Health Percent ----------
UEnvQueryTest_CompanionHealth::UEnvQueryTest_CompanionHealth(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	SetWorkOnFloatValues(true); 
}

void UEnvQueryTest_CompanionHealth::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return;
	}

	// the filter range from the editor (only used when the test filters)
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	const float MinThreshold = FloatValueMin.GetValue();
	const float MaxThreshold = FloatValueMax.GetValue();

	const FGameplayTag HealthTag = TagDefinitions::Stats_Resources_Health;
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const AEstivalBaseCharacter* Enemy = Cast<AEstivalBaseCharacter>(GetItemActor(QueryInstance, It.GetIndex()));
		const UResourceComponent* Resources = Enemy ? Enemy->GetResources() : nullptr;

		// 0 = almost dead, 1 = full health. No Health at all counts as full,
		// because GetPercentage01 returns 0 for it, which would look "almost dead"
		const bool bHasHealth = Resources && Resources->HasResource(HealthTag);
		const float Health = bHasHealth ? Resources->GetPercentage01(HealthTag) : 1.f;
		It.SetScore(TestPurpose, FilterType, Health, MinThreshold, MaxThreshold);
	}
}

// ---------- Companion Is After a Player ----------
UEnvQueryTest_CompanionAfterPlayer::UEnvQueryTest_CompanionAfterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	SetWorkOnFloatValues(false); // a yes/no test
}

void UEnvQueryTest_CompanionAfterPlayer::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return;
	}

	// Bool Match in the editor: which answer passes
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	const bool bWanted = BoolValue.GetValue();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// the enemy's controller remembers who it chases 
		const APawn* Enemy = Cast<APawn>(GetItemActor(QueryInstance, It.GetIndex()));
		const AEnemyAIController* EnemyAI = Enemy ? Cast<AEnemyAIController>(Enemy->GetController()) : nullptr;
		const bool bAfterPlayer = EnemyAI && EnemyAI->PlayerRef && EnemyAI->PlayerRef->IsPlayerControlled();
		It.SetScore(TestPurpose, FilterType, bAfterPlayer, bWanted);
	}
}

// ---------- Companion Has Actor Tag ----------
UEnvQueryTest_CompanionHasTag::UEnvQueryTest_CompanionHasTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	SetWorkOnFloatValues(false);
}

void UEnvQueryTest_CompanionHasTag::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return;
	}

	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	const bool bWanted = BoolValue.GetValue();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// Actor Tags are plain names so no enemy code has to change
		const AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		const bool bHasTag = ItemActor && ItemActor->ActorHasTag(Tag);
		It.SetScore(TestPurpose, FilterType, bHasTag, bWanted);
	}
}