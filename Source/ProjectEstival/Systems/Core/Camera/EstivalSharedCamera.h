// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EstivalSharedCamera.generated.h"

class AGameModeBase;
class APlayerController;
class UCameraComponent;
class USpringArmComponent;
class UEstivalOffscreenBlipsWidget;

UCLASS()
class PROJECTESTIVAL_API AEstivalSharedCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEstivalSharedCamera();
	
	virtual void Tick(float DeltaTime) override;
	
	// Pick an actor so the camera centers on it (eg Baldr's Light)
	// To center back to the party pass nullptr
	UFUNCTION(BlueprintCallable, Category = "Shared Camera")
	void SetFocusOnTarget(AActor* NewFocusTarget);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;
	
	// actor to center on. When empty: center of all framed characters.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shared Camera")
	TObjectPtr<AActor> FocusTarget;
	
	// Prefered distnce from the focus point. Camera never gets closer than this
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Distance")
	float MinDistance = 800.0f;
	
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Distance")
	float MaxDistance = 2500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Distance", meta = (ClampMin = "0", Units = "cm"))
	float ScreenEdgePadding = 150.0f;
	
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Smoothing", meta = (ClampMin = "0"))
	float FollowSpeed = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Smoothing", meta = (ClampMin = "0"))
	float ZoomOutSpeed = 6.f;
	
	// when "0" = instant
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Smoothing", meta = (ClampMin = "0"))
	float ZoomInSpeed = 1.5f;
	
	/** Draws the focus point and each framed character (green = fits at max distance, red = doesn't). */
	UPROPERTY(EditAnywhere, Category = "Shared Camera|Debug")
	bool bDrawDebug = false;
	
	//Widget that shows blips for off-screen players. Empty = no blips.
	// todo: when the main HUD is ready the blips can move there and this can be deleted (I think)
	UPROPERTY(EditAnywhere, Category = "Shared Camera|UI")
	TSubclassOf<UEstivalOffscreenBlipsWidget> OffscreenBlipsWidgetClass;

private:
	void UpdateCamera(float DeltaSeconds, bool bSnap);
	void GatherFramedActors(TArray<AActor*>& OutActors) const;
	FVector ComputeFocusLocation(const TArray<AActor*>& Actors) const;
	float ComputeRequiredDistance(const FVector& FocusLocation, const TArray<AActor*>& Actors) const;
	float DistanceToFit(const FVector& FocusLocation, const FVector& WorldLocation) const;
	float GetViewportAspectRatio() const;

	void TakeOverview(APlayerController* PlayerController);
	void HandlePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);

	FDelegateHandle PostLoginHandle;
	
	// the blips widget this camera created, so EndPlay can remove it
	UPROPERTY()
	TObjectPtr<UEstivalOffscreenBlipsWidget> BlipsWidget;
	
	// True if the location would still be on screen with the camera at Max Distance. 
	// Movement can use this to keep players inside the frame.
	bool CanFrameLocation(const FVector& WorldLocation) const;
	
};
