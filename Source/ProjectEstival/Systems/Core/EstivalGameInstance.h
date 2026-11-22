#pragma once

#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "Engine/GameInstance.h"
#include "CoreMinimal.h"

#include "EstivalGameInstance.generated.h"

class APlayerBase;

UCLASS()
class PROJECTESTIVAL_API UEstivalGameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly)
	TSet<TSubclassOf<APlayerBase>> PlayerHeroes;
	
	UPROPERTY(EditDefaultsOnly, meta = (MustImplement = "/Script/ProjectEstival.Poolable"))
	TMap<TSubclassOf<AActor>, FActorPoolSettings> ActorPoolSettings;
	
public:
	FActorPoolSettings GetActorPoolSettings(const TSubclassOf<AActor> ActorClass) const
	{
		return ActorPoolSettings.FindRef(ActorClass);
	}
	
	TSet<TSubclassOf<APlayerBase>> GetHeroClasses() const
	{
		return PlayerHeroes;
	}
};
