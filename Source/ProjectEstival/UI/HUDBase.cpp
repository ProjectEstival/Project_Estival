#include "HUDBase.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!BaseWidgetClass) return;
	
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	BaseWidget = CreateWidget<UUserWidget>(PC, BaseWidgetClass);
	if (!BaseWidget) return;
	
	BaseWidget->AddToViewport();
	BaseWidget->SetVisibility(ESlateVisibility::Visible);
	ApplyInputMode(BaseWidget, BaseInputMode, bBaseShowCursor, PC);
}
UUserWidget* AHUDBase::CreateOrGetWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC) const
{
	if (!WidgetClass || WidgetClass == BaseWidgetClass) return nullptr;
	if (UUserWidget* PresentWidget = GetWidget(WidgetClass)) return PresentWidget;
    
	UWorld* World = GetWorld();
	if (!World) return nullptr;
    
	APlayerController* owner = !PC ? GetGameInstance()->GetFirstLocalPlayerController() : PC;
	UUserWidget* Widget = CreateWidget<UUserWidget>(owner, WidgetClass);
	if (!Widget) return nullptr;
	return Widget;
}

UUserWidget* AHUDBase::PushWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder, EManagedUIInputMode InputMode, bool bShowMouseCursor, APlayerController* PC)
{
	if (!WidgetClass) return nullptr;
	
	// Remove stale entries of the same class.
	ModalStack.RemoveAll([WidgetClass](const FWidgetContext& Entry) { return Entry.WidgetClass == WidgetClass && !IsValid(Entry.Widget); });
	
	// If it already exists in the stack, return nullptr
	if (ModalStack.FindByPredicate([WidgetClass](const FWidgetContext& Entry) { return Entry.WidgetClass == WidgetClass; })) return nullptr;
    
	UUserWidget* Widget = CreateOrGetWidget(WidgetClass, PC);
	if (!Widget) return nullptr;
    
	FWidgetContext Entry;
	Entry.WidgetClass = WidgetClass;
	Entry.Widget = Widget;
	Entry.InputMode = InputMode;
	Entry.bShowMouseCursor = bShowMouseCursor;
	Entry.PC = PC;
	ModalStack.Add(Entry);
    
	if (!Widget->IsInViewport()) Widget->AddToViewport(ZOrder);
	Widget->SetVisibility(ESlateVisibility::Visible);
	ApplyInputMode(Widget, InputMode, bShowMouseCursor, PC);
	return Widget;
}
UUserWidget* AHUDBase::PopWidget()
{
	if (ModalStack.IsEmpty()) return nullptr;
    
	// Remove the top modal.
	FWidgetContext ClosedModal = ModalStack.Pop();
	if (IsValid(ClosedModal.Widget)) ClosedModal.Widget->RemoveFromParent();
    
	// Restore the previous modal and its input settings.
	while (!ModalStack.IsEmpty())
	{
		const FWidgetContext& Top = ModalStack.Last();
		if (IsValid(Top.Widget))
		{
			Top.Widget->SetVisibility(ESlateVisibility::Visible);
			ApplyInputMode(Top.Widget, Top.InputMode, Top.bShowMouseCursor, Top.PC);
			return Top.Widget;
		}
        
		// Discard stale entries if a widget was destroyed.
		ModalStack.Pop();
	}
    
	// No modals remain: return to normal gameplay input.
	ApplyInputMode(nullptr, EManagedUIInputMode::GameOnly, false);
	return nullptr;
}

UUserWidget* AHUDBase::PeekWidget()
{
	while (!ModalStack.IsEmpty())
	{
		const FWidgetContext Top = ModalStack.Last();
		if (IsValid(Top.Widget)) return Top.Widget;
		ModalStack.Pop();
	}
	return nullptr;
}
UUserWidget* AHUDBase::GetWidget(TSubclassOf<UUserWidget> WidgetClass) const
{
	const FWidgetContext* Top = ModalStack.FindByPredicate([WidgetClass](const FWidgetContext& Entry) { return Entry.WidgetClass == WidgetClass; });
	if (!Top) return nullptr;
	return Top->Widget;
}
EManagedUIInputMode AHUDBase::GetCurrentInputMode()
{
	while (!ModalStack.IsEmpty())
	{
		const FWidgetContext& Top = ModalStack.Last();
		if (IsValid(Top.Widget)) return Top.InputMode;
		ModalStack.Pop();
	}
	return EManagedUIInputMode::GameAndUI;
}

void AHUDBase::ApplyInputMode(UUserWidget* Widget, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC, const bool bDisableOthers)
{
	if (bDisableOthers)
	{
		// Disable all controllers but the one provided 
		for (int i = 0; i < GetGameInstance()->GetNumLocalPlayers(); i++)
		{
			APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), i);
			Controller->DisableInput(Controller);
		}	
	}
	
	// If no controller is given, apply input mode one by one, without disabling the others
	if (PC) PC->EnableInput(PC);
	else
	{
		for (int i = 0; i < GetGameInstance()->GetNumLocalPlayers(); i++)
		{
			ApplyInputMode(Widget, InputMode, bShowMouseCursor, UGameplayStatics::GetPlayerController(GetWorld(), i), false);
		}
		return;
	}
	
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
