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