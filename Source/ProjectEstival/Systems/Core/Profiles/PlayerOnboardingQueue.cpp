#include "Systems/Core/Profiles/PlayerOnboardingQueue.h"

#include "Engine/LocalPlayer.h"

void UPlayerOnboardingQueue::SetMandatoryPlayer(ULocalPlayer* LocalPlayer)
{
	MandatoryPlayer = LocalPlayer;
}
bool UPlayerOnboardingQueue::IsMandatoryPlayer(const ULocalPlayer* LocalPlayer) const
{
	return IsValid(LocalPlayer) && MandatoryPlayer.Get() == LocalPlayer;
}

void UPlayerOnboardingQueue::Enqueue(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer) || PendingPlayers.Contains(LocalPlayer) || CurrentPlayer == LocalPlayer) return;
	PendingPlayers.Add(LocalPlayer);
	AdvanceIfIdle();
}
void UPlayerOnboardingQueue::CompleteCurrent(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer) || CurrentPlayer != LocalPlayer) return;
	CurrentPlayer = nullptr;
	// The HUD closes the completed player's popup before calling Continue()
}
void UPlayerOnboardingQueue::CancelCurrent(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer) || CurrentPlayer != LocalPlayer) return;
	CurrentPlayer = nullptr;
	AdvanceIfIdle();
}
void UPlayerOnboardingQueue::RemovePlayer(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return;
	PendingPlayers.RemoveAll([LocalPlayer](const TObjectPtr<ULocalPlayer>& Player) { return Player.Get() == LocalPlayer; });
	if (CurrentPlayer == LocalPlayer) CurrentPlayer = nullptr;
	if (MandatoryPlayer.Get() == LocalPlayer) MandatoryPlayer.Reset();
	AdvanceIfIdle();
}
void UPlayerOnboardingQueue::Continue()
{
	AdvanceIfIdle();
}

void UPlayerOnboardingQueue::AdvanceIfIdle()
{
	if (IsValid(CurrentPlayer)) return;
	while (!PendingPlayers.IsEmpty())
	{
		ULocalPlayer* NextPlayer = PendingPlayers[0];
		PendingPlayers.RemoveAt(0);
		if (!IsValid(NextPlayer)) continue;
		CurrentPlayer = NextPlayer;
		OnCurrentPlayerChanged.Broadcast(NextPlayer);
		return;
	}
}
