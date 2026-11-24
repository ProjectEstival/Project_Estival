#include "UI/InGame/PlayerCard.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/Characters/PlayerBase.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "Systems/Mike/Combat/Loadouts/Loadout.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void UPlayerCard::InitializeCard(APlayerBase* InPlayer, const int32 InPartyIndex, const FLinearColor& InPlayerColor)
{
	Player = InPlayer;
	PartyIndex = InPartyIndex;
	
	if (Player && Player->GetResources()) Player->GetResources()->OnResourceChanged.AddDynamic(this, &UPlayerCard::OnResourceChanged);
	
	const ULoadoutComponent* LoadoutComponent = Player ? Player->GetLoadout() : nullptr;
	const ULoadout* Loadout = LoadoutComponent ? LoadoutComponent->GetLoadout() : nullptr;
	if (CharacterTitle) CharacterTitle->SetText(Loadout ? Loadout->DisplayName : FText::GetEmpty());
	
	static const FName AbilitySlotNames[] = { TEXT("Ability.First"), TEXT("Ability.Second"), TEXT("Ability.Third"), TEXT("Ability.Fourth"), TEXT("Ability.Potion") };
	AbilitySlotTags.Reset(UE_ARRAY_COUNT(AbilitySlotNames));
	for (int32 AbilityIndex = 0; AbilityIndex < UE_ARRAY_COUNT(AbilitySlotNames); ++AbilityIndex)
	{
		const FGameplayTag SlotTag = FGameplayTag::RequestGameplayTag(AbilitySlotNames[AbilityIndex], false);
		AbilitySlotTags.Add(SlotTag);
		const UAbilityDefinition* Ability = Loadout && SlotTag.IsValid() ? Loadout->Abilities.FindRef(SlotTag) : nullptr;
		SetAbilityIcon(AbilityIndex, Ability ? Ability->Icon : nullptr);
	}
	UpdateAbilityCooldowns();
	
	FText ProfileName = FText::FromString(TEXT("AI"));
	const FLinearColor ProfileColor = InPlayerColor;
	const UGameInstance* GameInstance = GetGameInstance();
	const APlayerController* PlayerController = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (const ULocalPlayerSessionDirectory* Directory = GameInstance ? GameInstance->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr; Directory && LocalPlayer)
	{
		const UPlayerProfileSubsystem* Profiles = GameInstance->GetSubsystem<UPlayerProfileSubsystem>();
		if (const UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr; Profile && !Profile->Profile.ProfileName.IsEmpty()) ProfileName = FText::FromString(Profile->Profile.ProfileName);
	}
	
	if (ProfileText) ProfileText->SetText(ProfileName);
	if (CardBorder) CardBorder->SetBrushColor(ProfileColor);
	if (TagBorder) TagBorder->SetBrushColor(ProfileColor);
	if (TagColor) TagColor->SetColorAndOpacity(ProfileColor);
}

void UPlayerCard::SetAbilityIcon(const int32 AbilityIndex, UTexture2D* Icon)
{
	const TArray<UImage*> AbilityImages = { Ability0, Ability1, Ability2, Ability3, Elixir };
	if (!AbilityImages.IsValidIndex(AbilityIndex) || !AbilityImages[AbilityIndex]) return;
	
	UImage* Image = AbilityImages[AbilityIndex];
	if (!Icon)
	{
		Image->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	
	if (!AbilityMaterials.IsValidIndex(AbilityIndex)) AbilityMaterials.SetNum(AbilityImages.Num());
	if (!AbilityMaterials[AbilityIndex])
	{
		UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(Image->GetBrush().GetResourceObject());
		if (!BaseMaterial)
		{
			Image->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		
		AbilityMaterials[AbilityIndex] = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (!AbilityMaterials[AbilityIndex])
		{
			Image->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		Image->SetBrushFromMaterial(AbilityMaterials[AbilityIndex]);
	}
	
	AbilityMaterials[AbilityIndex]->SetTextureParameterValue(TEXT("Texture"), Icon);
	Image->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerCard::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateAbilityCooldowns();
}

void UPlayerCard::UpdateAbilityCooldowns()
{
	const ULoadoutComponent* LoadoutComponent = Player ? Player->GetLoadout() : nullptr;
	if (!LoadoutComponent) return;
	
	for (int32 AbilityIndex = 0; AbilityIndex < AbilitySlotTags.Num(); ++AbilityIndex)
	{
		if (!AbilityMaterials.IsValidIndex(AbilityIndex) || !AbilityMaterials[AbilityIndex]) continue;
		
		const FGameplayTag& SlotTag = AbilitySlotTags[AbilityIndex];
		const float Threshold = SlotTag.IsValid() && LoadoutComponent->IsOnCooldown(SlotTag) ? LoadoutComponent->GetCooldownRemaining01(SlotTag) : 0.0f;
		AbilityMaterials[AbilityIndex]->SetScalarParameterValue(TEXT("Threshold"), Threshold);
	}
}

void UPlayerCard::NativeDestruct()
{
	if (Player && Player->GetResources()) Player->GetResources()->OnResourceChanged.RemoveDynamic(this, &UPlayerCard::OnResourceChanged);
	
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
