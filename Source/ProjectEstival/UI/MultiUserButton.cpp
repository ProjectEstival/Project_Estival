#include "UI/MultiUserButton.h"

#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "Components/ButtonSlot.h"
#include "Widgets/Input/SButton.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

class SMultiUserButton final : public SButton
{
public:
	uint32 GetCurrentInputUserIndex() const { return CurrentInputUserIndex; }
	
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, FocusEvent.GetUser());
		return SButton::OnFocusReceived(MyGeometry, FocusEvent);
	}
	virtual void OnFocusLost(const FFocusEvent& FocusEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, FocusEvent.GetUser());
		SButton::OnFocusLost(FocusEvent);
	}
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, KeyEvent.GetUserIndex());
		return SButton::OnKeyDown(MyGeometry, KeyEvent);
	}
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, KeyEvent.GetUserIndex());
		return SButton::OnKeyUp(MyGeometry, KeyEvent);
	}
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
	}
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		return SButton::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
	}
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
	}
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		return SButton::OnMouseMove(MyGeometry, MouseEvent);
	}
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		SButton::OnMouseEnter(MyGeometry, MouseEvent);
	}
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, MouseEvent.GetUserIndex());
		SButton::OnMouseLeave(MouseEvent);
	}
	virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override
	{
		TGuardValue<uint32> UserScope(CurrentInputUserIndex, static_cast<uint32>(CaptureLostEvent.UserIndex));
		SButton::OnMouseCaptureLost(CaptureLostEvent);
	}
	
private:
	uint32 CurrentInputUserIndex = MAX_uint32;
};

TSharedRef<SWidget> UMultiUserButton::RebuildWidget()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	MultiUserSlateButton = SNew(SMultiUserButton).OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandleSlateClicked)).OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleSlatePressed)).OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleSlateReleased)).OnHovered_UObject(this, &UMultiUserButton::HandleSlateHovered).OnUnhovered_UObject(this, &UMultiUserButton::HandleSlateUnhovered).OnReceivedFocus_UObject(this, &UMultiUserButton::HandleSlateFocusReceived).OnLostFocus_UObject(this, &UMultiUserButton::HandleSlateFocusLost).OnSlateButtonDragDetected(BIND_UOBJECT_DELEGATE(FOnDragDetected, SlateHandleDragDetected)).OnSlateButtonDragEnter(BIND_UOBJECT_DELEGATE(FOnDragEnter, SlateHandleDragEnter)).OnSlateButtonDragLeave(BIND_UOBJECT_DELEGATE(FOnDragLeave, SlateHandleDragLeave)).OnSlateButtonDragOver(BIND_UOBJECT_DELEGATE(FOnDragOver, SlateHandleDragOver)).OnSlateButtonDrop(BIND_UOBJECT_DELEGATE(FOnDrop, SlateHandleDrop)).ButtonStyle(&WidgetStyle).ClickMethod(ClickMethod).TouchMethod(TouchMethod).PressMethod(PressMethod).IsFocusable(IsFocusable).AllowDragDrop(bAllowDragDrop);
	MyButton = MultiUserSlateButton;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	if (GetChildrenCount() > 0) Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	
	return MyButton.ToSharedRef();
}

void UMultiUserButton::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MultiUserSlateButton.Reset();
	FocusedUserIndices.Reset();
	HoveredUserIndices.Reset();
}

TArray<ULocalPlayer*> UMultiUserButton::GetFocusedPlayers() const
{
	return GetLocalPlayersForUsers(FocusedUserIndices);
}
TArray<ULocalPlayer*> UMultiUserButton::GetHoveredPlayers() const
{
	return GetLocalPlayersForUsers(HoveredUserIndices);
}

TArray<ULocalPlayer*> UMultiUserButton::GetLocalPlayersForUsers(const TSet<uint32>& UserIndices) const
{
	TArray<ULocalPlayer*> Result;
	const UGameInstance* GI = GetGameInstance();
	if (!GI) return Result;
	
	for (ULocalPlayer* LocalPlayer : GI->GetLocalPlayers())
	{
		if (!IsValid(LocalPlayer)) continue;
		const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser();
		if (SlateUser.IsValid() && UserIndices.Contains(static_cast<uint32>(SlateUser->GetUserIndex()))) Result.Add(LocalPlayer);
	}
	return Result;
}
APlayerController* UMultiUserButton::GetPlayerControllerForUser(const uint32 UserIndex) const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI) return nullptr;
	for (ULocalPlayer* LocalPlayer : GI->GetLocalPlayers())
	{
		if (!IsValid(LocalPlayer)) continue;
		const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser();
		if (SlateUser.IsValid() && static_cast<uint32>(SlateUser->GetUserIndex()) == UserIndex) return LocalPlayer->GetPlayerController(GetWorld());
	}
	return nullptr;
}

uint32 UMultiUserButton::GetCurrentEventUserIndex() const
{
	return MultiUserSlateButton.IsValid() ? MultiUserSlateButton->GetCurrentInputUserIndex() : MAX_uint32;
}

FReply UMultiUserButton::HandleSlateClicked()
{
	const FReply Reply = SlateHandleClicked();
	OnClickedByPlayer.Broadcast(GetPlayerControllerForUser(GetCurrentEventUserIndex()));
	return Reply;
}
void UMultiUserButton::HandleSlatePressed()
{
	SlateHandlePressed();
	OnPressedByPlayer.Broadcast(GetPlayerControllerForUser(GetCurrentEventUserIndex()));
}
void UMultiUserButton::HandleSlateReleased()
{
	SlateHandleReleased();
	OnReleasedByPlayer.Broadcast(GetPlayerControllerForUser(GetCurrentEventUserIndex()));
}
void UMultiUserButton::HandleSlateHovered()
{
	const uint32 UserIndex = GetCurrentEventUserIndex();
	HoveredUserIndices.Add(UserIndex);
	SlateHandleHovered();
	OnHoveredByPlayer.Broadcast(GetPlayerControllerForUser(UserIndex));
}
void UMultiUserButton::HandleSlateUnhovered()
{
	const uint32 UserIndex = GetCurrentEventUserIndex();
	HoveredUserIndices.Remove(UserIndex);
	SlateHandleUnhovered();
	OnUnhoveredByPlayer.Broadcast(GetPlayerControllerForUser(UserIndex));
}
void UMultiUserButton::HandleSlateFocusReceived()
{
	const uint32 UserIndex = GetCurrentEventUserIndex();
	FocusedUserIndices.Add(UserIndex);
	SlateHandleOnReceivedFocus();
	OnFocusedByPlayer.Broadcast(GetPlayerControllerForUser(UserIndex));
}
void UMultiUserButton::HandleSlateFocusLost()
{
	const uint32 UserIndex = GetCurrentEventUserIndex();
	FocusedUserIndices.Remove(UserIndex);
	SlateHandleOnLostFocus();
	OnFocusLostByPlayer.Broadcast(GetPlayerControllerForUser(UserIndex));
}
