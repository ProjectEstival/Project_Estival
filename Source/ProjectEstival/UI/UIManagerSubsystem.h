#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class EManagedUIInputMode : uint8
{
    GameOnly,
    GameAndUI,
    UIOnly
};

USTRUCT()
struct FManagedModal
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
};

UCLASS()
class PROJECTESTIVAL_API UUIManagerSubsystem  : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    // Create once per class and show. Reuses existing instances.
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0, EManagedUIInputMode InputMode = EManagedUIInputMode::GameOnly, bool bShowMouseCursor = false);
    
    // Hide without destroying the instance.
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void HideWidget(TSubclassOf<UUserWidget> WidgetClass);
    
    // Remove from viewport and forget the instance.
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void RemoveWidget(TSubclassOf<UUserWidget> WidgetClass);
    
    // Replace the active primary screen.
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    UUserWidget* SwitchScreen(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0, EManagedUIInputMode InputMode = EManagedUIInputMode::UIOnly, bool bShowMouseCursor = true);
    
    // Retrieve only an existing widget.
    UFUNCTION(BlueprintPure, Category = "UI Manager")
    UUserWidget* GetWidget(TSubclassOf<UUserWidget> WidgetClass) const;
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void RemoveAllWidgets();
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    UUserWidget* PushModal(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 100, EManagedUIInputMode InputMode = EManagedUIInputMode::GameAndUI, bool bShowMouseCursor = true);
    
    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void PopModal();
    
    virtual void Deinitialize() override;
        
private:
    UPROPERTY(Transient)
    TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> WidgetInstances;
    
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveScreen;
    
    UPROPERTY(Transient)
    TArray<FManagedModal> ModalStack;
    
    UUserWidget* CreateOrGetWidget(TSubclassOf<UUserWidget> WidgetClass);
    void ApplyInputMode(UUserWidget* Widget, EManagedUIInputMode InputMode, bool bShowMouseCursor);
};