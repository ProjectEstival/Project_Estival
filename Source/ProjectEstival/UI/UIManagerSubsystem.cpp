#include "UIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"

UUserWidget* UUIManagerSubsystem::CreateOrGetWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass) return nullptr;
    if (TObjectPtr<UUserWidget>* Found = WidgetInstances.Find(WidgetClass))
    {
        if (IsValid(Found->Get())) return Found->Get();
        WidgetInstances.Remove(WidgetClass);
    }
    
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC) return nullptr;
    
    UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!Widget) return nullptr;
    
    WidgetInstances.Add(WidgetClass, Widget);
    return Widget;
}

UUserWidget* UUIManagerSubsystem::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder, EManagedUIInputMode InputMode, bool bShowMouseCursor)
{
    UUserWidget* Widget = CreateOrGetWidget(WidgetClass);
    if (!Widget) return nullptr;
    
    if (!Widget->IsInViewport()) Widget->AddToViewport(ZOrder);
    Widget->SetVisibility(ESlateVisibility::Visible);
    ApplyInputMode(Widget, InputMode, bShowMouseCursor);
    return Widget;
}

void UUIManagerSubsystem::HideWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (UUserWidget* Widget = GetWidget(WidgetClass))
    {
        Widget->SetVisibility(ESlateVisibility::Hidden);
        if (ActiveScreen == Widget) ActiveScreen = nullptr;
    }
}

void UUIManagerSubsystem::RemoveWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (UUserWidget* Widget = GetWidget(WidgetClass))
    {
        Widget->RemoveFromParent();
        if (ActiveScreen == Widget) ActiveScreen = nullptr;
    }
    WidgetInstances.Remove(WidgetClass);
}

UUserWidget* UUIManagerSubsystem::SwitchScreen(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder, EManagedUIInputMode InputMode, bool bShowMouseCursor)
{
    if (!WidgetClass) return nullptr;
    
    // Same screen requested again: don't recreate it.
    if (ActiveScreen && ActiveScreen->GetClass() == WidgetClass)
    {
        ActiveScreen->SetVisibility(ESlateVisibility::Visible);
        ApplyInputMode(ActiveScreen, InputMode, bShowMouseCursor);
        return ActiveScreen;
    }
    
    if (IsValid(ActiveScreen)) ActiveScreen->SetVisibility(ESlateVisibility::Hidden);
    UUserWidget* NewScreen = ShowWidget(WidgetClass, ZOrder, InputMode, bShowMouseCursor);
    
    if (NewScreen) ActiveScreen = NewScreen;
    return NewScreen;
}

UUserWidget* UUIManagerSubsystem::GetWidget(TSubclassOf<UUserWidget> WidgetClass) const
{
    if (const TObjectPtr<UUserWidget>* Found = WidgetInstances.Find(WidgetClass)) return Found->Get();
    return nullptr;
}

void UUIManagerSubsystem::RemoveAllWidgets()
{
    for (auto& Entry : WidgetInstances)
    {
        if (IsValid(Entry.Value)) Entry.Value->RemoveFromParent();
    }
    
    ModalStack.Empty();
    WidgetInstances.Empty();
    ActiveScreen = nullptr;
    ApplyInputMode(nullptr, EManagedUIInputMode::GameOnly, false);
}

UUserWidget* UUIManagerSubsystem::PushModal(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder, EManagedUIInputMode InputMode, bool bShowMouseCursor)
{
    if (!WidgetClass) return nullptr;
    
    UUserWidget* Widget = CreateOrGetWidget(WidgetClass);
    if (!Widget) return nullptr;
    
    // Prevent duplicate stack entries for the same widget.
    // If it already exists in the stack, move it to the top.
    ModalStack.RemoveAll([WidgetClass](const FManagedModal& Entry){ return Entry.WidgetClass == WidgetClass; });
    
    FManagedModal Entry;
    Entry.WidgetClass = WidgetClass;
    Entry.Widget = Widget;
    Entry.InputMode = InputMode;
    Entry.bShowMouseCursor = bShowMouseCursor;
    ModalStack.Add(Entry);
    
    if (!Widget->IsInViewport()) Widget->AddToViewport(ZOrder);
    Widget->SetVisibility(ESlateVisibility::Visible);
    ApplyInputMode(Widget, InputMode, bShowMouseCursor);
    return Widget;
}

void UUIManagerSubsystem::PopModal()
{
    if (ModalStack.IsEmpty()) return;
    
    // Remove the top modal.
    FManagedModal ClosedModal = ModalStack.Pop();
    if (IsValid(ClosedModal.Widget)) ClosedModal.Widget->SetVisibility(ESlateVisibility::Hidden);
    
    // Restore the previous modal and its input settings.
    while (!ModalStack.IsEmpty())
    {
        const FManagedModal& Top = ModalStack.Last();
        if (IsValid(Top.Widget))
        {
            Top.Widget->SetVisibility(ESlateVisibility::Visible);
            ApplyInputMode(Top.Widget, Top.InputMode, Top.bShowMouseCursor);
            return;
        }
        
        // Discard stale entries if a widget was destroyed.
        ModalStack.Pop();
    }
    
    // No modals remain: return to normal gameplay input.
    ApplyInputMode(nullptr, EManagedUIInputMode::GameOnly, false);
}

void UUIManagerSubsystem::Deinitialize()
{
    RemoveAllWidgets();
    Super::Deinitialize();
}

void UUIManagerSubsystem::ApplyInputMode(UUserWidget* Widget, EManagedUIInputMode InputMode, bool bShowMouseCursor)
{
    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC) return;
    
    PC->bShowMouseCursor = bShowMouseCursor;
    switch (InputMode)
    {
        case EManagedUIInputMode::GameOnly:
        {
            FInputModeGameOnly Mode;
            PC->SetInputMode(Mode);
            break;
        }
        
        case EManagedUIInputMode::GameAndUI:
        {
            FInputModeGameAndUI Mode;
            if (IsValid(Widget)) Mode.SetWidgetToFocus(Widget->TakeWidget());
            PC->SetInputMode(Mode);
            break;
        }
        
        case EManagedUIInputMode::UIOnly:
        {
            FInputModeUIOnly Mode;
            if (IsValid(Widget)) Mode.SetWidgetToFocus(Widget->TakeWidget());
            PC->SetInputMode(Mode);
            break;
        }
    }
}