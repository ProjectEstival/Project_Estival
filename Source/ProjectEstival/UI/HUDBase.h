#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDBase.generated.h"

UENUM(BlueprintType)
enum class EManagedUIInputMode : uint8
{
	GameOnly,
	GameAndUI,
	UIOnly
};

USTRUCT()
struct FWidgetContext
{
	GENERATED_BODY()
    
	UPROPERTY()
	TSubclassOf<UUserWidget> WidgetClass;
    
	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;
    
	UPROPERTY()
	EManagedUIInputMode InputMode = EManagedUIInputMode::GameAndUI;
    
	UPROPERTY()
	bool bShowMouseCursor = true;
	
	UPROPERTY()
	APlayerController* PC = nullptr;
};

UCLASS()
class PROJECTESTIVAL_API AHUDBase : public AHUD
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* PushWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 100, EManagedUIInputMode InputMode = EManagedUIInputMode::GameAndUI, bool bShowMouseCursor = true, APlayerController* PC = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* PopWidget();
	
	// Retrieve the top widget.
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	UUserWidget* PeekWidget();
	
	// Retrieve only an existing widget.
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	UUserWidget* GetWidget(TSubclassOf<UUserWidget> WidgetClass) const;
	
	// Retrieve the top widget's input mode
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	EManagedUIInputMode GetCurrentInputMode();
	
protected:	
	UPROPERTY(Transient)
	TArray<FWidgetContext> ModalStack;
	
	UUserWidget* CreateOrGetWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC);
	void ApplyInputMode(UUserWidget* Widget, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC = nullptr, const bool bDisableOthers = true);
};
