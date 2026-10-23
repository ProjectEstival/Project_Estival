#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;

UCLASS()
class PROJECTESTIVAL_API UUIManagerSubsystem  : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:    
    virtual void Deinitialize() override;
        
private:
    
};