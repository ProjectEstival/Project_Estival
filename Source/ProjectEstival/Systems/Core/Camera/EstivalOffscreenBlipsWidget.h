// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstivalOffscreenBlipsWidget.generated.h"

class UCanvasPanel;
class UImage;
class UTexture2D;
class AEstivalBaseCharacter;
class ULoadout;

/**
* Shows a colored blip on the edge of the screen for every human player who is currently off screen.
* Adds it to the screen once (not once per player).
*/
UCLASS()
class PROJECTESTIVAL_API UEstivalOffscreenBlipsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// Full-screen canvas the blips are placed on. Must be called "BlipCanvas" in the Widget Blueprint. 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> BlipCanvas;
	
	// Picture used for every blip. Empty = a plain square. 
	UPROPERTY(EditAnywhere, Category="Blips")
	TObjectPtr<UTexture2D>  BlipTexture;
	
	UPROPERTY(EditAnywhere, Category="Blips")
	FVector2D BlipSize = FVector2D(32.f, 32.f);
	
	// Gap between a blip and the screen edge.
	UPROPERTY(EditAnywhere, Category="Blips", meta = (ClampMin = "0"))
	float EdgeMargin = 40.f;
	
	UPROPERTY(EditAnywhere, Category = "Blips")
	TMap<TObjectPtr<ULoadout>, FLinearColor> HeroColors;

	
private:
	UImage* GetOrCreateBlip(int32 Index);
	void UpdateBlip(UImage* Blip, const APawn* Pawn, const FLinearColor& Color, APlayerController* ViewingPlayer, const FVector2D& ScreenSize);
	
	UPROPERTY()
	TArray<TObjectPtr<UImage>> Blips;
	
	
	
};
