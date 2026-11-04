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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnboardingEvent, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileAssigned, ULocalPlayer*, LocalPlayer, FGuid, ProfileId);


UCLASS()
class PROJECTESTIVAL_API UPlayerProfileSubsystem  : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnboardingEvent OnOnboardingPlayerChanged;
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnboardingEvent OnProfileSelectionCancelRequested;
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnProfileAssigned OnProfileAssigned;
    
    // Profiles
    FGuid CreateProfile(const FString& ProfileName);
    TArray<FProfileSummary> GetUnassignedProfiles() const;
    
    // Onboarding
    UFUNCTION()
    void CancelProfileSelection(ULocalPlayer* LocalPlayer);
    bool ConfirmProfileSelection(ULocalPlayer* LocalPlayer, const FGuid& ProfileId);
    void ContinueOnboarding();
    bool IsPlayerRegistered(ULocalPlayer* LocalPlayer) const;
    
private:
    int32 MaxPlayers = 4;
    FString SavePrefix = TEXT("Profile_");
    
    // Input
    friend class FPlayerJoinInputProcessor;
    TSharedPtr<FPlayerJoinInputProcessor> InputProcessor;
    bool HandlePreprocessorKeyDown(const FKeyEvent& KeyEvent);
    void HandleJoinInput(FInputDeviceId InputDeviceId);
    bool HandleCancelInput(FInputDeviceId InputDeviceId) const;
    
    // Onboarding
    UPROPERTY()
    TArray<TObjectPtr<ULocalPlayer>> PendingPlayers;
    UPROPERTY(Transient)
    TArray<TObjectPtr<ULocalPlayer>> TemporaryPlayers;
    UPROPERTY()
    TObjectPtr<ULocalPlayer> CurrentOnboardingPlayer;
    void EnqueuePlayer(ULocalPlayer* LocalPlayer);
    void ShowNextOnboardingPlayer();
    
    // Save / load
    UPROPERTY(EditDefaultsOnly, Category = "Profiles")
    TSubclassOf<UPlayerProfileSaveGame> ProfileSaveGameClass = UPlayerProfileSaveGame::StaticClass();
    UPROPERTY(Transient)
    TArray<FProfileSummary> SavedProfiles;
    void RefreshSavedProfiles();
    UPROPERTY()
    TMap<FGuid, TObjectPtr<UPlayerProfileSaveGame>> LoadedProfiles;
    bool LoadProfile(const FGuid& ProfileId);
    bool SaveProfile(const FGuid& ProfileId) const;
    UPlayerProfileSaveGame* GetLoadedProfile(const FGuid& ProfileId) const;
    
    // Profile selection
    UPROPERTY()
    TMap<TObjectPtr<ULocalPlayer>, FGuid> SelectedProfiles;
    bool IsProfileTaken(const FGuid& ProfileId) const;
    bool AssignProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId);
    void ReleaseProfile(ULocalPlayer* LocalPlayer);
    
    // Helpers
    FString GetProfileSlotName(const FGuid& ProfileId) const;
};