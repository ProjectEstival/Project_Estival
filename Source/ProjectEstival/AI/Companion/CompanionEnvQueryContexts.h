#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "CompanionEnvQueryContexts.generated.h"

/** The player this companion follows (from ACompanionAIController) */
UCLASS()
class PROJECTESTIVAL_API UEnvQueryContext_CompanionFollowTarget : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/** Where the other companions stand, or where they are walking to */
UCLASS()
class PROJECTESTIVAL_API UEnvQueryContext_OtherCompanions : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/** The enemy this companion fights */
UCLASS()
class PROJECTESTIVAL_API UEnvQueryContext_CompanionAttackTarget : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};