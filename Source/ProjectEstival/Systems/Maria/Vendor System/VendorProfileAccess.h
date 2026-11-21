#pragma once

#include "CoreMinimal.h"

class AActor;
class ULocalPlayer;
class UPlayerProfileSaveGame;
class UPlayerProfileSubsystem;

/** The saved profile of the local player behind a buyer. The profile wallet and the profile grants both go through it,
 * so a buyer is always resolved the same way. Short-lived: resolve, use, drop. Never store it.
 * */
struct FVendorProfileAccess
{
#pragma region Public Methods
	/** Accepts the hero pawn or its player controller. Returns an invalid access for bots and anything without a local player. */
	static FVendorProfileAccess Find(AActor* Buyer);

	bool IsValid() const { return Save != nullptr; }

	/** Writes the profile to disk. Call after every change to Save. */
	void Commit() const;
#pragma endregion

	UPlayerProfileSaveGame* Save = nullptr;

private:
#pragma region Private Fields
	const UPlayerProfileSubsystem* Profiles = nullptr;
	ULocalPlayer* LocalPlayer = nullptr;
#pragma endregion
};