#include "HUDBase.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"

void AHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!BaseWidgetClass) return;
	APlayerController* PC = GetOwningPlayerController();
	if (!IsValid(PC)) return;
	
	// The root presentation is shared-screen UI. Create it once, for player 0
	const UGameInstance* GI = GetGameInstance();
	if (!GI || PC->GetLocalPlayer() != GI->GetLocalPlayerByIndex(0)) return;
	
	BaseWidget = CreateWidget<UUserWidget>(PC, BaseWidgetClass);
	if (!BaseWidget) return;
	BaseWidget->AddToViewport();
	BaseWidget->SetVisibility(ESlateVisibility::Visible);
	// The shared root widget belongs to player 0; do not give its focus to another Slate user
	ApplyInputMode(BaseWidget, BaseInputMode, bBaseShowCursor, PC);
}

UUserWidget* AHUDBase::CreateOrGetWidget(const TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC) const
{
	if (!WidgetClass || WidgetClass == BaseWidgetClass) return nullptr;
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC)) return nullptr;
	return CreateWidget<UUserWidget>(PC, WidgetClass);
}
UUserWidget* AHUDBase::PushWidget(TSubclassOf<UUserWidget> WidgetClass, const int32 ZOrder, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC)
{
	if (!WidgetClass) return nullptr;
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetLocalPlayer())) return nullptr;
	
	TArray<FWidgetContext>& Stack = PlayerModalStacks.FindOrAdd(PC->GetLocalPlayer()).Contexts;
	Stack.RemoveAll([WidgetClass](const FWidgetContext& Entry) { return Entry.WidgetClass == WidgetClass && !IsValid(Entry.Widget); });
	if (Stack.ContainsByPredicate([WidgetClass](const FWidgetContext& Entry) { return Entry.WidgetClass == WidgetClass; })) return nullptr;
	
	UUserWidget* Widget = CreateOrGetWidget(WidgetClass, PC);
	if (!Widget) return nullptr;
	
	FWidgetContext Entry;
	Entry.WidgetClass = WidgetClass;
	Entry.Widget = Widget;
	Entry.InputMode = InputMode;
	Entry.bShowMouseCursor = bShowMouseCursor;
	Entry.PC = PC;
	if (!Stack.IsEmpty())
	{
		const FWidgetContext& Previous = Stack.Last();
		Entry.PreviousFocusWidget = Previous.Widget;
		Entry.PreviousInputMode = Previous.InputMode;
		Entry.bPreviousShowMouseCursor = Previous.bShowMouseCursor;
	}
	else
	{
		Entry.PreviousFocusWidget = PC->GetLocalPlayer()->GetLocalPlayerIndex() == 0 ? BaseWidget : nullptr;
		Entry.PreviousInputMode = BaseInputMode;
		Entry.bPreviousShowMouseCursor = bBaseShowCursor;
	}
	Stack.Add(Entry);
	
	if (!Widget->IsInViewport()) Widget->AddToViewport(ZOrder);
	Widget->SetVisibility(ESlateVisibility::Visible);
	ApplyInputMode(Widget, InputMode, bShowMouseCursor, PC);
	return Widget;
}
UUserWidget* AHUDBase::PopWidget(APlayerController* PC)
{
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetLocalPlayer())) return nullptr;
	
	FPlayerWidgetStack* FoundStack = PlayerModalStacks.Find(PC->GetLocalPlayer());
	if (!FoundStack || FoundStack->Contexts.IsEmpty()) return nullptr;
	TArray<FWidgetContext>& Stack = FoundStack->Contexts;
	
	FWidgetContext Closed = Stack.Pop();
	if (IsValid(Closed.Widget)) Closed.Widget->RemoveFromParent();
	while (!Stack.IsEmpty())
	{
		const FWidgetContext& Top = Stack.Last();
		if (!IsValid(Top.Widget)) { Stack.Pop(); continue; }
		Top.Widget->SetVisibility(ESlateVisibility::Visible);
		ApplyInputMode(Top.Widget, Top.InputMode, Top.bShowMouseCursor, PC);
		return Top.Widget;
	}
	
	// The shared root widget belongs to player 0; restore focus to it only for that user
	ApplyInputMode(Closed.PreviousFocusWidget, Closed.PreviousInputMode, Closed.bPreviousShowMouseCursor, PC);
	PlayerModalStacks.Remove(PC->GetLocalPlayer());
	return nullptr;
}
UUserWidget* AHUDBase::PeekWidget(APlayerController* PC)
{
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetLocalPlayer())) return nullptr;
	FPlayerWidgetStack* PlayerStack = PlayerModalStacks.Find(PC->GetLocalPlayer());
	TArray<FWidgetContext>* Stack = PlayerStack ? &PlayerStack->Contexts : nullptr;
	while (Stack && !Stack->IsEmpty())
	{
		if (IsValid(Stack->Last().Widget)) return Stack->Last().Widget;
		Stack->Pop();
	}
	return nullptr;
}
UUserWidget* AHUDBase::GetWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC) const
{
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetLocalPlayer())) return nullptr;
	const FPlayerWidgetStack* PlayerStack = PlayerModalStacks.Find(PC->GetLocalPlayer());
	if (!PlayerStack) return nullptr;
	const FWidgetContext* Entry = PlayerStack->Contexts.FindByPredicate([WidgetClass](const FWidgetContext& Item) { return Item.WidgetClass == WidgetClass; });
	return Entry ? Entry->Widget.Get() : nullptr;
}

EManagedUIInputMode AHUDBase::GetCurrentInputMode(APlayerController* PC)
{
	if (!IsValid(PC)) PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetLocalPlayer())) return BaseInputMode;
	FPlayerWidgetStack* PlayerStack = PlayerModalStacks.Find(PC->GetLocalPlayer());
	TArray<FWidgetContext>* Stack = PlayerStack ? &PlayerStack->Contexts : nullptr;
	while (Stack && !Stack->IsEmpty())
	{
		if (IsValid(Stack->Last().Widget)) return Stack->Last().InputMode;
		Stack->Pop();
	}
	return BaseInputMode;
}
void AHUDBase::ApplyInputMode(UUserWidget* Widget, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC) const
{
	if (!IsValid(PC)) return;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>()) Directory->SetFocusedWidget(PC->GetLocalPlayer(), Widget);
	}
	PC->bShowMouseCursor = bShowMouseCursor;
	switch (InputMode)
	{
		case EManagedUIInputMode::GameOnly:
			PC->SetInputMode(FInputModeGameOnly());
			break;
		case EManagedUIInputMode::GameAndUI:
		{
			FInputModeGameAndUI Mode;
			if (IsValid(Widget) && Widget->IsFocusable()) Mode.SetWidgetToFocus(Widget->TakeWidget());
			PC->SetInputMode(Mode);
			break;
		}
		case EManagedUIInputMode::UIOnly:
		{
			FInputModeUIOnly Mode;
			if (IsValid(Widget) && Widget->IsFocusable()) Mode.SetWidgetToFocus(Widget->TakeWidget());
			PC->SetInputMode(Mode);
			break;
		}
	}
}
