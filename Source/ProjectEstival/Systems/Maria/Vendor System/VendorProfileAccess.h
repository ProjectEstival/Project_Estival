#pragma once

#include "CoreMinimal.h"

class AActor;
class ULocalPlayer;
class UPlayerProfileSaveGame;
class UPlayerProfileSubsystem;

/** The saved profile of the local player behind a buyer. The profile grants go through it, and the vendor uses FindLocalPlayer to charge,
 * so a buyer is always resolved the same way. Short-lived: resolve, use, drop. Never store it.
 * */
struct FVendorProfileAccess
{
#pragma region Public Methods
	/** The local player behind a hero pawn or its player controller. Null for bots and anything else. */
	static ULocalPlayer* FindLocalPlayer(AActor* Buyer);

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