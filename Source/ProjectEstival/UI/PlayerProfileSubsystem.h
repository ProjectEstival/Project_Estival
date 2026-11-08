#pragma once

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "CoreMinimal.h"
#include "PlayerProfileSubsystem.generated.h"

class FPlayerJoinInputProcessor;
class APlayerController;
class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnboardingPlayerChanged, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileAssigned, ULocalPlayer*, LocalPlayer, int32, ProfileIndex);

UCLASS()
class PROJECTESTIVAL_API UPlayerProfileSubsystem  : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnOnboardingPlayerChanged OnOnboardingPlayerChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnProfileAssigned OnProfileAssigned;
    
    bool IsPlayerRegistered(ULocalPlayer* LocalPlayer) const;
    UFUNCTION(BlueprintCallable, Category = "Profiles")
	void RequestPlayerJoin(FPlatformUserId PlatformUserId);
    
    void ConfirmProfileSelection(ULocalPlayer* LocalPlayer, int32 ProfileIndex);
    UFUNCTION()
    void CancelProfileSelection(ULocalPlayer* LocalPlayer);
    
private:
    friend class FPlayerJoinInputProcessor;
    TSharedPtr<FPlayerJoinInputProcessor> InputProcessor;
    bool HandlePreprocessorKeyDown(const FKeyEvent& KeyEvent);
    void HandleJoinInput(FInputDeviceId InputDeviceId);
    
    int32 MaxPlayers = 4;
    
    UPROPERTY()
    TArray<TObjectPtr<ULocalPlayer>> PendingPlayers;
    
    UPROPERTY()
    TObjectPtr<ULocalPlayer> CurrentOnboardingPlayer;
    
    UPROPERTY()
    TMap<TObjectPtr<ULocalPlayer>, int32> SelectedProfiles;
        
    void EnqueuePlayer(ULocalPlayer* LocalPlayer);
    void ShowNextOnboardingPlayer();
    
    UPROPERTY(Transient)
    TArray<TObjectPtr<ULocalPlayer>> JoinCreatedPlayers;
};