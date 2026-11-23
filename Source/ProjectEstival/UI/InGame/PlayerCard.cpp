#include "UI/InGame/PlayerCard.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/Characters/PlayerBase.h"
#include "Systems/Mike/Combat/Loadouts/Loadout.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void UPlayerCard::InitializeCard(APlayerBase* InPlayer, const int32 InPartyIndex, const FLinearColor& InPlayerColor)
{
	Player = InPlayer;
	PartyIndex = InPartyIndex;
	
	Player->GetResources()->OnResourceChanged.AddDynamic(this, &UPlayerCard::OnResourceChanged);
	
	if (CharacterTitle)
	{
		const ULoadoutComponent* LoadoutComponent = Player ? Player->GetLoadout() : nullptr;
		const ULoadout* Loadout = LoadoutComponent ? LoadoutComponent->GetLoadout() : nullptr;
		CharacterTitle->SetText(Loadout ? Loadout->DisplayName : FText::GetEmpty());
	}
	
	FText ProfileName = FText::FromString(TEXT("AI"));
	const FLinearColor ProfileColor = InPlayerColor;
	const UGameInstance* GameInstance = GetGameInstance();
	const APlayerController* PlayerController = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (const ULocalPlayerSessionDirectory* Directory = GameInstance ? GameInstance->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr; Directory && LocalPlayer)
	{
		const UPlayerProfileSubsystem* Profiles = GameInstance->GetSubsystem<UPlayerProfileSubsystem>();
		const UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr;
		if (Profile && !Profile->Profile.ProfileName.IsEmpty()) ProfileName = FText::FromString(Profile->Profile.ProfileName);
	}
	
	if (ProfileText) ProfileText->SetText(ProfileName);
	if (CardBorder) CardBorder->SetBrushColor(ProfileColor);
	if (TagBorder) TagBorder->SetBrushColor(ProfileColor);
	if (TagColor) TagColor->SetColorAndOpacity(ProfileColor);
}

void UPlayerCard::NativeDestruct()
{
	Player->GetResources()->OnResourceChanged.RemoveDynamic(this, &UPlayerCard::OnResourceChanged);
	
	Super::NativeDestruct();
}

void UPlayerCard::OnResourceChanged(const FGameplayTag Stat, const float NewValue, const float OldValue, const float MaxValue) const
{	
	if (Stat.MatchesTagExact(TagDefinitions::Stats_Resources_Health) && HealthBar)
	{
		UE_LOG(LogTemp, Warning, TEXT("Health"));
		HealthBar.Get()->SetPercent(NewValue / MaxValue);
	}
	else if (Stat.MatchesTagExact(TagDefinitions::Stats_Resources_Corruption) && CorruptionBar)
	{
		UE_LOG(LogTemp, Warning, TEXT("Corruption"));
		CorruptionBar.Get()->SetPercent(NewValue / MaxValue);
	}
}
