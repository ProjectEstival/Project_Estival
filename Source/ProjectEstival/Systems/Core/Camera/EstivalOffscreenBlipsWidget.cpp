// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Core/Camera/EstivalOffscreenBlipsWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Mike/Combat/Loadouts/Loadout.h"

void UEstivalOffscreenBlipsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* ViewingPlayer = GetOwningPlayer();
	if (!ViewingPlayer || !BlipCanvas)
	{
		return;
	}

	const FVector2D ScreenSize = MyGeometry.GetLocalSize();
	int32 BlipIndex = 0;

	// find all heroes
	for (TActorIterator<AEstivalBaseCharacter> It(GetWorld()); It; ++It)
	{
		const AEstivalBaseCharacter* Character = *It;

		// enemies are on another team and dead heroes get no blip
		if (Character->GetGenericTeamId() != FGenericTeamId(0) || !Character->IsAlive())
		{
			continue;
		}

		// the hero's loadout picks the color. Currently no loadout or no color means no blip
		const ULoadoutComponent* LoadoutComponent = Character->FindComponentByClass<ULoadoutComponent>();
		const ULoadout* Hero = LoadoutComponent ? LoadoutComponent->GetLoadout() : nullptr;
		if (!Hero)
		{
			continue;
		}

		UpdateBlip(GetOrCreateBlip(BlipIndex), Character, Hero->LoadoutColor, ViewingPlayer, ScreenSize);
		BlipIndex++;
	}

	// hide unused blips
	for (int32 i = BlipIndex; i < Blips.Num(); i++)
	{
		Blips[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}


UImage* UEstivalOffscreenBlipsWidget::GetOrCreateBlip(int32 Index)
{
	// make a blip the first time a hero needs one then reuse it
	if (!Blips.IsValidIndex(Index))
	{
		UImage* NewBlip = WidgetTree->ConstructWidget<UImage>();
		NewBlip->SetBrushFromTexture(BlipTexture); 

		UCanvasPanelSlot* BlipSlot = BlipCanvas->AddChildToCanvas(NewBlip);
		BlipSlot->SetSize(BlipSize);
		
		// the position shoule be the middle of the blip, not its top left corner
		BlipSlot->SetAlignment(FVector2D(0.5f, 0.5f));

		Blips.Add(NewBlip);
	}
	return Blips[Index];
}

void UEstivalOffscreenBlipsWidget::UpdateBlip(UImage* Blip, const APawn* Pawn, const FLinearColor& Color, APlayerController* ViewingPlayer, const FVector2D& ScreenSize)
{
	FVector2D ScreenPosition;
	const bool bInFrontOfCamera = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(ViewingPlayer, Pawn->GetActorLocation(), ScreenPosition, false);

	// behind the camera the projection fails. Our camera looks down and never turns, 
	// so "behind" means below the bottom edge
	if (!bInFrontOfCamera)
	{
		ScreenPosition = FVector2D(ScreenSize.X * 0.5f, ScreenSize.Y + 1.f);
	}

	const bool bOnScreen = ScreenPosition.X >= 0.f && ScreenPosition.X <= ScreenSize.X && ScreenPosition.Y >= 0.f && ScreenPosition.Y <= ScreenSize.Y;
	if (bOnScreen)
	{
		Blip->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// pin the blip to the edge of the screen
	const FVector2D BlipPosition(
		FMath::Clamp(ScreenPosition.X, EdgeMargin, ScreenSize.X - EdgeMargin),
		FMath::Clamp(ScreenPosition.Y, EdgeMargin, ScreenSize.Y - EdgeMargin));

	CastChecked<UCanvasPanelSlot>(Blip->Slot)->SetPosition(BlipPosition);

	Blip->SetColorAndOpacity(Color);
	Blip->SetVisibility(ESlateVisibility::HitTestInvisible);
}