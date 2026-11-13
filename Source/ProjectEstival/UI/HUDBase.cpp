#include "HUDBase.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
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
	UWidget* InitialFocusWidget = FindDefaultFocusableWidget(BaseWidget);
	ApplyInputMode(InitialFocusWidget ? InitialFocusWidget : BaseWidget, BaseInputMode, bBaseShowCursor, PC);
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
	if (Stack.IsEmpty())
	{
		FWidgetContext BaseContext;
		BaseContext.WidgetClass = BaseWidgetClass;
		BaseContext.Widget = GetSharedBaseWidget();
		BaseContext.InputMode = BaseInputMode;
		BaseContext.bShowMouseCursor = bBaseShowCursor;
		BaseContext.PC = PC;
		BaseContext.bIsBaseContext = true;
		Stack.Add(BaseContext);
	}
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
	check(!Stack.IsEmpty()); // The per-player base context must remain below every modal.
	const FWidgetContext& Previous = Stack.Last();
	Entry.PreviousFocusWidget = FindFocusedWidgetIn(Previous.Widget, PC);
	if (!IsValid(Entry.PreviousFocusWidget)) Entry.PreviousFocusWidget = FindDefaultFocusableWidget(Previous.Widget);
	if (!IsValid(Entry.PreviousFocusWidget)) Entry.PreviousFocusWidget = Previous.Widget;
	Entry.PreviousInputMode = Previous.InputMode;
	Entry.bPreviousShowMouseCursor = Previous.bShowMouseCursor;
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
	if (!FoundStack || FoundStack->Contexts.Num() <= 1) return nullptr; // The base context cannot be popped.
	TArray<FWidgetContext>& Stack = FoundStack->Contexts;

	const FWidgetContext Closed = Stack.Pop();
	if (IsValid(Closed.Widget)) Closed.Widget->RemoveFromParent();
	while (!Stack.IsEmpty())
	{
		const FWidgetContext& Top = Stack.Last();
		if (!Top.bIsBaseContext && !IsValid(Top.Widget)) { Stack.Pop(); continue; }
		if (IsValid(Top.Widget)) Top.Widget->SetVisibility(ESlateVisibility::Visible);
		UWidget* FocusWidget = IsValid(Closed.PreviousFocusWidget) ? Closed.PreviousFocusWidget.Get() : FindDefaultFocusableWidget(Top.Widget);
		if (!IsValid(FocusWidget)) FocusWidget = Top.Widget.Get();
		ApplyInputMode(FocusWidget, Closed.PreviousInputMode, Closed.bPreviousShowMouseCursor, PC);
		return Top.Widget.Get();
	}
	
	// Restore the exact element focused before this modal was pushed.
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
		if (Stack->Last().bIsBaseContext) return Stack->Last().Widget.Get();
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
		if (Stack->Last().bIsBaseContext) return Stack->Last().InputMode;
		if (IsValid(Stack->Last().Widget)) return Stack->Last().InputMode;
		Stack->Pop();
	}
	return BaseInputMode;
}
UUserWidget* AHUDBase::GetSharedBaseWidget() const
{
	if (IsValid(BaseWidget)) return BaseWidget;
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayer* FirstPlayer = GI ? GI->GetLocalPlayerByIndex(0) : nullptr;
	const APlayerController* FirstPC = FirstPlayer ? FirstPlayer->GetPlayerController(GetWorld()) : nullptr;
	const AHUDBase* FirstHUD = FirstPC ? Cast<AHUDBase>(FirstPC->GetHUD()) : nullptr;
	return IsValid(FirstHUD) ? FirstHUD->BaseWidget.Get() : nullptr;
}
UWidget* AHUDBase::FindFocusedWidgetIn(const UUserWidget* Scope, const APlayerController* PC)
{
	if (!IsValid(Scope) || !IsValid(PC) || !IsValid(PC->GetLocalPlayer()) || !FSlateApplication::IsInitialized()) return nullptr;
	const TSharedPtr<FSlateUser> SlateUser = PC->GetLocalPlayer()->GetSlateUser();
	if (!SlateUser.IsValid() || !Scope->WidgetTree) return nullptr;
	const TSharedPtr<SWidget> FocusedSlateWidget = FSlateApplication::Get().GetUserFocusedWidget(SlateUser->GetUserIndex());
	if (!FocusedSlateWidget.IsValid()) return nullptr;
	UWidget* Result = nullptr;
	Scope->WidgetTree->ForEachWidget([&](UWidget* Candidate) { if (IsValid(Candidate) && Candidate->GetCachedWidget() == FocusedSlateWidget) Result = Candidate; });
	return Result;
}
UWidget* AHUDBase::FindDefaultFocusableWidget(const UUserWidget* Scope)
{
	if (!IsValid(Scope) || !Scope->WidgetTree) return nullptr;
	UWidget* DesiredFocusWidget = Scope->GetDesiredFocusWidget();
	auto IsUsableFocusTarget = [](UWidget* Candidate)
	{
		if (!IsValid(Candidate) || !Candidate->IsVisible() || !Candidate->GetIsEnabled()) return false;
		const TSharedPtr<SWidget> SlateWidget = Candidate->TakeWidget();
		return SlateWidget.IsValid() && SlateWidget->SupportsKeyboardFocus();
	};
	if (IsUsableFocusTarget(DesiredFocusWidget)) return DesiredFocusWidget;

	UWidget* FirstFocusableWidget = nullptr;
	Scope->WidgetTree->ForEachWidget([&](UWidget* Candidate) { if (!FirstFocusableWidget && IsUsableFocusTarget(Candidate)) FirstFocusableWidget = Candidate; });
	return FirstFocusableWidget;
}

void AHUDBase::ApplyInputMode(UWidget* FocusWidget, const EManagedUIInputMode InputMode, const bool bShowMouseCursor, APlayerController* PC) const
{
	if (!IsValid(PC)) return;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>()) Directory->SetFocusedWidget(PC->GetLocalPlayer(), FocusWidget);
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
			if (IsValid(FocusWidget))
			{
				const TSharedPtr<SWidget> SlateFocusWidget = FocusWidget->TakeWidget();
				if (SlateFocusWidget.IsValid() && SlateFocusWidget->SupportsKeyboardFocus()) Mode.SetWidgetToFocus(SlateFocusWidget);
			}
			PC->SetInputMode(Mode);
			break;
		}
		case EManagedUIInputMode::UIOnly:
		{
			FInputModeUIOnly Mode;
			if (IsValid(FocusWidget))
			{
				const TSharedPtr<SWidget> SlateFocusWidget = FocusWidget->TakeWidget();
				if (SlateFocusWidget.IsValid() && SlateFocusWidget->SupportsKeyboardFocus()) Mode.SetWidgetToFocus(SlateFocusWidget);
			}
			PC->SetInputMode(Mode);
			break;
		}
	}
}
