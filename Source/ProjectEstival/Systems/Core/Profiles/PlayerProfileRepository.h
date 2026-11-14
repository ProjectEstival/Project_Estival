#pragma once

#include "PlayerProfileData.h"
#include "UObject/Object.h"
#include "CoreMinimal.h"

#include "PlayerProfileRepository.generated.h"

class UPlayerProfileSaveGame;

/** Owns profile save slots and loaded SaveGame objects. Contains no player or UI flow. */
UCLASS()
class PROJECTESTIVAL_API UPlayerProfileRepository : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize();
	FGuid CreateProfile(const FString& ProfileName);
	bool EditProfileName(const FGuid& ProfileId, const FString& NewName);
	bool DeleteProfile(const FGuid& ProfileId);
	const TArray<FProfileSummary>& GetSavedProfiles() const { return SavedProfiles; }
	bool LoadProfile(const FGuid& ProfileId);
	bool SaveProfile(const FGuid& ProfileId) const;
	UPlayerProfileSaveGame* GetLoadedProfile(const FGuid& ProfileId) const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Profiles")
	TSubclassOf<UPlayerProfileSaveGame> ProfileSaveGameClass;
	UPROPERTY(Transient)
	TArray<FProfileSummary> SavedProfiles;
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UPlayerProfileSaveGame>> LoadedProfiles;
	FString SavePrefix = TEXT("Profile_");
	void RefreshSavedProfiles();
	FString GetProfileSlotName(const FGuid& ProfileId) const;
};
