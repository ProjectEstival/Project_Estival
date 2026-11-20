
#include "GameFramework/Actor.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/GameModes/LevelManager.h"
#include "A_LevelManager.generated.h"

UCLASS()
class PROJECTESTIVAL_API AA_LevelManager : public AActor, public ILevelManager
{
	GENERATED_BODY()
public:
	virtual void BuildFloor_Implementation(int32 Floor) override;
protected:
	void FinishFloor(FTransform StartTransform, AEstivalBaseCharacter *Boss);
	//void ClearCurrentFloor();
};
