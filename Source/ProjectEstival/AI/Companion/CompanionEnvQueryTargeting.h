#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "CompanionEnvQueryTargeting.generated.h"

/** Every enemy in this companion's fight (from ACompanionAIController, filled by Find Threat) */
UCLASS(meta=(DisplayName="Companion Threats"))
class PROJECTESTIVAL_API UEnvQueryGenerator_CompanionThreats : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_CompanionThreats(const FObjectInitializer& ObjectInitializer);

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
};

/** Health from 0 (almost dead) to 1 (full). Score it Inverse Linear to prefer weak enemies */
UCLASS(meta=(DisplayName="Companion: Health Percent"))
class PROJECTESTIVAL_API UEnvQueryTest_CompanionHealth : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CompanionHealth(const FObjectInitializer& ObjectInitializer);

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
/** Yes if this enemy is chasing a human player. Score it to protect the players */
UCLASS(meta=(DisplayName="Companion: Is After a Player"))
class PROJECTESTIVAL_API UEnvQueryTest_CompanionAfterPlayer : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CompanionAfterPlayer(const FObjectInitializer& ObjectInitializer);

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};

// Yes if the actor has this tag in Details › Actor › Tags, for example Caster 
UCLASS(meta=(DisplayName="Companion: Has Actor Tag"))
class PROJECTESTIVAL_API UEnvQueryTest_CompanionHasTag : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CompanionHasTag(const FObjectInitializer& ObjectInitializer);

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	// the tag to look for. Designers add it to an enemy in Details › Actor › Tags
	UPROPERTY(EditDefaultsOnly, Category=Test)
	FName Tag = TEXT("Caster");
};