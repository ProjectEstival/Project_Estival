// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Core/Camera/EstivalOffscreenBlipsWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UEstivalOffscreenBlipsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// the player this widget belongs to 
	APlayerController* ViewingPlayer = GetOwningPlayer();
	if (!ViewingPlayer || !BlipCanvas)
	{
		return;
	}

	const FVector2D ScreenSize = MyGeometry.GetLocalSize();
	int32 BlipIndex = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;

		UImage* Blip = GetOrCreateBlip(BlipIndex);
		BlipIndex++;

		// dead or not spawned yet: no blip 
		// todo: make sure we need this. might remote later
		if (!Pawn)
		{
			Blip->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		// where the player is on the screen in widget units 
		FVector2D ScreenPosition;
		const bool bInFrontOfCamera = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(ViewingPlayer, Pawn->GetActorLocation(), ScreenPosition, false);

		// behind the camera the projection fails. Our camera looks down and never turns,
		// so "behind" always means below the bottom edge
		if (!bInFrontOfCamera)
		{
			ScreenPosition = FVector2D(ScreenSize.X * 0.5f, ScreenSize.Y + 1.f);
		}

		const bool bOnScreen = ScreenPosition.X >= 0.f && ScreenPosition.X <= ScreenSize.X
			&& ScreenPosition.Y >= 0.f && ScreenPosition.Y <= ScreenSize.Y;

		if (bOnScreen)
		{
			Blip->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		// pin the blip to the screen edge on the side where the player is
		const FVector2D BlipPosition(
			FMath::Clamp(ScreenPosition.X, EdgeMargin, ScreenSize.X - EdgeMargin),
			FMath::Clamp(ScreenPosition.Y, EdgeMargin, ScreenSize.Y - EdgeMargin));

		if (UCanvasPanelSlot* BlipSlot = Cast<UCanvasPanelSlot>(Blip->Slot))
		{
			BlipSlot->SetPosition(BlipPosition);
		}
		Blip->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// a player left the game: hide the blips nobody uses any more
	for (int32 i = BlipIndex; i < Blips.Num(); i++)
	{
		Blips[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}


UImage* UEstivalOffscreenBlipsWidget::GetOrCreateBlip(int32 Index)
{
	// only make a new blip the first time a player needs one and  then reuse it
	while (Blips.Num() <= Index)
	{
		const int32 NewIndex = Blips.Num();

		UImage* NewBlip = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (BlipTexture)
		{
			NewBlip->SetBrushFromTexture(BlipTexture);
		}
		NewBlip->SetColorAndOpacity(PlayerColors.IsValidIndex(NewIndex) ? PlayerColors[NewIndex] : FLinearColor::White);

		UCanvasPanelSlot* BlipSlot = BlipCanvas->AddChildToCanvas(NewBlip);
		BlipSlot->SetSize(BlipSize);
		// the position is the middle of the blip, not its top-left corner
		BlipSlot->SetAlignment(FVector2D(0.5f, 0.5f));

		Blips.Add(NewBlip);
	}
	return Blips[Index];
}