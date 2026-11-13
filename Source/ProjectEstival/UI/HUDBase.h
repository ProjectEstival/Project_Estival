#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/HUD.h"
#include "HUDBase.generated.h"

class ULocalPlayer;

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
	TObjectPtr<APlayerController> PC = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> PreviousFocusWidget = nullptr;

	UPROPERTY()
	EManagedUIInputMode PreviousInputMode = EManagedUIInputMode::GameAndUI;

	UPROPERTY()
	bool bPreviousShowMouseCursor = false;
};

USTRUCT()
struct FPlayerWidgetStack
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	TArray<FWidgetContext> Contexts;
};

UCLASS()
class PROJECTESTIVAL_API AHUDBase : public AHUD
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "UI Manager")
	TObjectPtr<UUserWidget> BaseWidget = nullptr;
	
	// Add widget to top
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* PushWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 100, EManagedUIInputMode InputMode = EManagedUIInputMode::GameAndUI, bool bShowMouseCursor = true, APlayerController* PC = nullptr);
	
	// Remove the top widget
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* PopWidget(APlayerController* PC = nullptr);
	
	// Retrieve the top widget
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	UUserWidget* PeekWidget(APlayerController* PC = nullptr);
	
	// Retrieve only an existing widget
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	UUserWidget* GetWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC = nullptr) const;
	
	// Retrieve the top widget's input mode
	UFUNCTION(BlueprintPure, Category = "UI Manager")
	EManagedUIInputMode GetCurrentInputMode(APlayerController* PC = nullptr);
	
protected:	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI Manager")
	TSubclassOf<UUserWidget> BaseWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI Manager")
	EManagedUIInputMode BaseInputMode;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI Manager")
	bool bBaseShowCursor;
	
	// Each local player owns an independent modal stack and input state.
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, FPlayerWidgetStack> PlayerModalStacks;
	
	UUserWidget* CreateOrGetWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC) const;
	void ApplyInputMode(UUserWidget* Widget, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC = nullptr) const;
};
