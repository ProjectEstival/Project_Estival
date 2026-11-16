#include "Systems/Core/Profiles/PlayerProfileRepository.h"

#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerProfileSaveGame.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

void UPlayerProfileRepository::Initialize()
{
	ProfileSaveGameClass = UPlayerProfileSaveGame::StaticClass();
	RefreshSavedProfiles();
}

void UPlayerProfileRepository::RefreshSavedProfiles()
{
	SavedProfiles.Reset();
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (!SaveSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Profiles] No save game system available"));
		return;
	}
	TArray<FString> SlotNames;
	SaveSystem->GetSaveGameNames(SlotNames, 0);
	TSet<FGuid> SeenProfileIds;
	for (const FString& SlotName : SlotNames)
	{
		if (!SlotName.StartsWith(SavePrefix)) continue;
		UPlayerProfileSaveGame* Save = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (!Save)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Profiles] Ignoring invalid save slot: %s"), *SlotName);
			continue;
		}
		const FGuid ProfileId = Save->Profile.ProfileId;
		if (!ProfileId.IsValid() || GetProfileSlotName(ProfileId) != SlotName || SeenProfileIds.Contains(ProfileId)) continue;
		SeenProfileIds.Add(ProfileId);
		FProfileSummary Summary;
		Summary.ProfileId = ProfileId;
		Summary.ProfileName = Save->Profile.ProfileName;
		SavedProfiles.Add(MoveTemp(Summary));
	}
	SavedProfiles.Sort([](const FProfileSummary& A, const FProfileSummary& B) { return A.ProfileName.Compare(B.ProfileName, ESearchCase::IgnoreCase) < 0; });
}

FGuid UPlayerProfileRepository::CreateProfile(const FString& ProfileName)
{
	const FString CleanName = ProfileName.TrimStartAndEnd();
	if (CleanName.IsEmpty() || !ProfileSaveGameClass) return FGuid();
	RefreshSavedProfiles();
	if (SavedProfiles.ContainsByPredicate([&CleanName](const FProfileSummary& P) { return P.ProfileName.Equals(CleanName, ESearchCase::IgnoreCase); })) return FGuid();
	const FGuid NewId = FGuid::NewGuid();
	UPlayerProfileSaveGame* Save = Cast<UPlayerProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(ProfileSaveGameClass));
	if (!Save) return FGuid();
	Save->Profile.ProfileId = NewId;
	Save->Profile.ProfileName = CleanName;
	if (!UGameplayStatics::SaveGameToSlot(Save, GetProfileSlotName(NewId), 0)) return FGuid();
	LoadedProfiles.Add(NewId, Save);
	RefreshSavedProfiles();
	return NewId;
}
bool UPlayerProfileRepository::EditProfileName(const FGuid& ProfileId, const FString& NewName)
{
	const FString CleanName = NewName.TrimStartAndEnd();
	if (!ProfileId.IsValid() || CleanName.IsEmpty()) return false;
	RefreshSavedProfiles();
	if (!SavedProfiles.ContainsByPredicate([&ProfileId](const FProfileSummary& P) { return P.ProfileId == ProfileId; })) return false;
	if (SavedProfiles.ContainsByPredicate([&CleanName, &ProfileId](const FProfileSummary& P) { return P.ProfileId != ProfileId && P.ProfileName.Equals(CleanName, ESearchCase::IgnoreCase); })) return false;
	UPlayerProfileSaveGame* Save = GetLoadedProfile(ProfileId);
	if (!Save && LoadProfile(ProfileId)) Save = GetLoadedProfile(ProfileId);
	if (!Save || Save->Profile.ProfileId != ProfileId) return false;
	if (Save->Profile.ProfileName == CleanName) return true;
	const FString OldName = Save->Profile.ProfileName;
	Save->Profile.ProfileName = CleanName;
	if (!SaveProfile(ProfileId))
	{
		Save->Profile.ProfileName = OldName;
		return false;
	}
	RefreshSavedProfiles();
	return true;
}
bool UPlayerProfileRepository::DeleteProfile(const FGuid& ProfileId)
{
	if (!ProfileId.IsValid()) return false;
	RefreshSavedProfiles();
	if (!SavedProfiles.ContainsByPredicate([&ProfileId](const FProfileSummary& P) { return P.ProfileId == ProfileId; })) return false;
	if (!UGameplayStatics::DeleteGameInSlot(GetProfileSlotName(ProfileId), 0)) return false;
	LoadedProfiles.Remove(ProfileId);
	RefreshSavedProfiles();
	return true;
}

bool UPlayerProfileRepository::LoadProfile(const FGuid& ProfileId)
{
	if (!ProfileId.IsValid()) return false;
	RefreshSavedProfiles();
	if (!SavedProfiles.ContainsByPredicate([&ProfileId](const FProfileSummary& P) { return P.ProfileId == ProfileId; })) return false;
	const FString SlotName = GetProfileSlotName(ProfileId);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return false;
	UPlayerProfileSaveGame* Save = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Save || Save->Profile.ProfileId != ProfileId) return false;
	LoadedProfiles.Add(ProfileId, Save);
	return true;
}
bool UPlayerProfileRepository::SaveProfile(const FGuid& ProfileId) const
{
	UPlayerProfileSaveGame* Save = GetLoadedProfile(ProfileId);
	return ProfileId.IsValid() && Save && Save->Profile.ProfileId == ProfileId && UGameplayStatics::SaveGameToSlot(Save, GetProfileSlotName(ProfileId), 0);
}

UPlayerProfileSaveGame* UPlayerProfileRepository::GetLoadedProfile(const FGuid& ProfileId) const
{
	const TObjectPtr<UPlayerProfileSaveGame>* Found = LoadedProfiles.Find(ProfileId);
	return Found ? Found->Get() : nullptr;
}
FString UPlayerProfileRepository::GetProfileSlotName(const FGuid& ProfileId) const
{
	return FString::Printf(TEXT("%s%s"), *SavePrefix, *ProfileId.ToString(EGuidFormats::Digits));
}
