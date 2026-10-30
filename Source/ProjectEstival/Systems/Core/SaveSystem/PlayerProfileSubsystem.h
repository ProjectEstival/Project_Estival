#pragma once

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerProfileSaveGame.h"
#include "InputCoreTypes.h"
#include "CoreMinimal.h"
#include "PlayerProfileSubsystem.generated.h"

class FPlayerJoinInputProcessor;
class APlayerController;
class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnboardingPlayerChanged, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileAssigned, ULocalPlayer*, LocalPlayer, FGuid, ProfileId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileSelectionCancelRequested, ULocalPlayer*, LocalPlayer);

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
    
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnProfileSelectionCancelRequested OnProfileSelectionCancelRequested;
    
    FGuid CreateProfile(const FString& ProfileName);
    TArray<FProfileSummary> GetAvailableProfiles() { return AvailableProfiles; }
    bool IsPlayerRegistered(ULocalPlayer* LocalPlayer) const;
    
    UFUNCTION()
    void CancelProfileSelection(ULocalPlayer* LocalPlayer);
    bool ConfirmProfileSelection(ULocalPlayer* LocalPlayer, const FGuid& ProfileId);
    
    void ContinueOnboarding();
    
private:
    friend class FPlayerJoinInputProcessor;
    TSharedPtr<FPlayerJoinInputProcessor> InputProcessor;
    bool HandlePreprocessorKeyDown(const FKeyEvent& KeyEvent);
    void HandleJoinInput(FInputDeviceId InputDeviceId);
    bool HandleCancelInput(FInputDeviceId InputDeviceId);
    
    int32 MaxPlayers = 4;
    
    UPROPERTY()
    TArray<TObjectPtr<ULocalPlayer>> PendingPlayers;
    UPROPERTY(Transient)
    TArray<TObjectPtr<ULocalPlayer>> TemporaryPlayers;
    UPROPERTY()
    TObjectPtr<ULocalPlayer> CurrentOnboardingPlayer;
    void EnqueuePlayer(ULocalPlayer* LocalPlayer);
    void ShowNextOnboardingPlayer();
       
    UPROPERTY()
    TMap<TObjectPtr<ULocalPlayer>, FGuid> SelectedProfiles;
    
    UPROPERTY()
    TMap<FGuid, TObjectPtr<UPlayerProfileSaveGame>> LoadedProfiles;
    
    UPROPERTY(EditDefaultsOnly, Category = "Profiles")
    TSubclassOf<UPlayerProfileSaveGame> ProfileSaveGameClass = UPlayerProfileSaveGame::StaticClass();
    
    UPROPERTY(Transient)
    TArray<FProfileSummary> AvailableProfiles;
    void RefreshAvailableProfiles();
    
    bool IsProfileTaken(const FGuid& ProfileId) const;
    bool AssignProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId);
    void ReleaseProfile(ULocalPlayer* LocalPlayer);
    
    bool LoadProfile(const FGuid& ProfileId);
    bool SaveProfile(const FGuid& ProfileId) const;
    
    UPlayerProfileSaveGame* GetLoadedProfile(const FGuid& ProfileId) const;
    static FString GetProfileSlotName(const FGuid& ProfileId);
};