#include "UI/ProfileDisplayEntry.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

bool UProfileDisplayEntry::SetPlayer(ULocalPlayer* LocalPlayer)
{
	Player = nullptr;
	ColorSlot = INDEX_NONE;
	if (IsValid(ProfileNameText)) ProfileNameText->SetText(FText::GetEmpty());
	if (!IsValid(LocalPlayer)) return false;
	
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	const FGuid ProfileId = Directory ? Directory->GetProfileId(LocalPlayer) : FGuid();
	const UPlayerProfileSaveGame* ProfileSave = Profiles && ProfileId.IsValid() ? Profiles->GetProfile(LocalPlayer) : nullptr;
	if (!ProfileSave || !IsValid(ProfileNameText)) return false;
	
	Player = LocalPlayer;
	ColorSlot = Directory->GetPlayerColorSlot(LocalPlayer);
	ProfileNameText->SetText(FText::FromString(ProfileSave->Profile.ProfileName));
	ProfileNameText->SetColorAndOpacity(FSlateColor(Directory->GetPlayerColor(LocalPlayer)));
	return true;
}
