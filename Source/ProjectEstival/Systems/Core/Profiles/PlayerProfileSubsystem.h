#pragma once

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerProfileSaveGame.h"
#include "InputCoreTypes.h"
#include "CoreMinimal.h"

#include "PlayerProfileSubsystem.generated.h"

class UPlayerProfileRepository;
class UPlayerOnboardingQueue;
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
    FOnboardingEvent OnProfileNameCommitRequested;
	UPROPERTY(BlueprintAssignable, Category = "Profiles")
	FOnboardingEvent OnPlayerDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Profiles")
	FOnboardingEvent OnProfileReleased;
    UPROPERTY(BlueprintAssignable, Category = "Profiles")
    FOnProfileAssigned OnProfileAssigned;
    
    // Profiles
    FGuid CreateProfile(const FString& ProfileName) const;
    bool EditProfileName(const FGuid& ProfileId, const FString& NewName) const;
    bool DeleteProfile(const FGuid& ProfileId) const;
    TArray<FProfileSummary> GetUnassignedProfiles() const;
    
    // Onboarding
    UFUNCTION()
    void CancelProfileSelection(ULocalPlayer* LocalPlayer);
    bool ConfirmProfileSelection(ULocalPlayer* LocalPlayer, const FGuid& ProfileId) const;
    void ContinueOnboarding() const;
    bool IsPlayerRegistered(ULocalPlayer* LocalPlayer) const;
	
	// Mandatory onboarding 
    void RequireProfileForPlayer(ULocalPlayer* LocalPlayer) const;
    bool IsMandatoryProfilePlayer(const ULocalPlayer* LocalPlayer) const;
    
	// Input
	void SetJoinInputEnabled(bool bEnabled); // Device joining is available only while the main-menu flow is active
    bool HandlePreprocessorJoinKeyDown(const FKeyEvent& KeyEvent) const; // Called by UPlayerJoinInputSubsystem after it filters the raw Slate key event
	bool HandleCancelInput(FInputDeviceId InputDeviceId) const;
    bool IsForeignKeyboardNameEntryActive() const;
    bool HandleForeignKeyboardNameCancel() const;
    bool TryCommitProfileNameFromDevice(FInputDeviceId InputDeviceId) const;
    void HandlePlayerJoined(ULocalPlayer* LocalPlayer, FInputDeviceId InputDeviceId, bool bCreatedForJoin);
	void HandleInputDeviceDisconnected(FInputDeviceId InputDeviceId);
    
    // Saving / loading
    UFUNCTION(BlueprintCallable, Category = "Profiles")
    UPlayerProfileSaveGame* GetProfile(ULocalPlayer* LocalPlayer) const;
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Profiles")
    bool SaveProfile(ULocalPlayer* LocalPlayer) const;
    
private:
    // Input
    bool bJoinInputEnabled = false;
    
    // Onboarding
    UPROPERTY(Transient)
    TArray<TObjectPtr<ULocalPlayer>> TemporaryPlayers;
    UPROPERTY(Transient)
    TObjectPtr<UPlayerOnboardingQueue> OnboardingQueue;
    void EnqueuePlayer(ULocalPlayer* LocalPlayer) const;
    UFUNCTION()
    void HandleOnboardingQueuePlayerChanged(ULocalPlayer* LocalPlayer) const;
    
    // Save / load
    UPROPERTY(Transient)
    TObjectPtr<UPlayerProfileRepository> ProfileRepository;
    
    // Profile selection
    bool IsProfileTaken(const FGuid& ProfileId) const;
    bool AssignProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId) const;
    void ReleaseProfile(ULocalPlayer* LocalPlayer) const;
};
