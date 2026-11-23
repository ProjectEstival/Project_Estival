
#include "CurrencySubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "../Core/Profiles/PlayerProfileSubsystem.h"
#include "../Core/Profiles/PlayerProfileSaveGame.h"

void UCurrencySubsystem::AddCurrency(ULocalPlayer* LocalPlayer,ECurrencyStorage Storage,int32 Amount)
{
    if (!LocalPlayer || Amount <= 0)
        return;

    UGameInstance* GI = GetGameInstance();
    UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;

    UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr;

    if (!Profile || !Profile->Profile.ProfileId.IsValid())
        return;

    if (Storage == ECurrencyStorage::Run)
    {
        const FGuid ProfileId = Profile->Profile.ProfileId;
        int32& Current = RunCurrencies.FindOrAdd(ProfileId);

        Current += Amount;
        OnCurrencyChanged.Broadcast(LocalPlayer, Storage, Current);
        return;
    }

    int32& Current = Profile->Profile.UnspentFragments;
    Current += Amount;

    if (!Profiles->SaveProfile(LocalPlayer))
    {
        Current -= Amount;
        return;
    }

    OnCurrencyChanged.Broadcast(LocalPlayer, Storage, Current);
}

int32 UCurrencySubsystem::GetCurrency(ULocalPlayer* LocalPlayer,ECurrencyStorage Storage) const
{
    if (!LocalPlayer)
        return 0;

    UGameInstance* GI = GetGameInstance();
    UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;

    UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr;

    if (!Profile || !Profile->Profile.ProfileId.IsValid())
        return 0;

    if (Storage == ECurrencyStorage::Run)
        return RunCurrencies.FindRef(Profile->Profile.ProfileId);

    return Profile->Profile.UnspentFragments;
}

bool UCurrencySubsystem::TryConsumeCurrency( ULocalPlayer* LocalPlayer, ECurrencyStorage Storage, int32 Amount)
{
    if (!LocalPlayer || Amount <= 0)
        return false;

    UGameInstance* GI = GetGameInstance();
    UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;

    UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr;

    if (!Profile || !Profile->Profile.ProfileId.IsValid())
        return false;

    if (Storage == ECurrencyStorage::Run)
    {
        const FGuid ProfileId = Profile->Profile.ProfileId;
        int32* Current = RunCurrencies.Find(ProfileId);

        if (!Current || *Current < Amount)
            return false;

        *Current -= Amount;
        OnCurrencyChanged.Broadcast(LocalPlayer, Storage, *Current);
        return true;
    }

    int32& Current = Profile->Profile.UnspentFragments;

    if (Current < Amount)
        return false;

    Current -= Amount;

    if (!Profiles->SaveProfile(LocalPlayer))
    {
        Current += Amount;
        return false;
    }

    OnCurrencyChanged.Broadcast(LocalPlayer, Storage, Current);
    return true;
}

void UCurrencySubsystem::ClearRunCurrency(ULocalPlayer* LocalPlayer)
{
    if (!LocalPlayer)
        return;

    UGameInstance* GI = GetGameInstance();
    UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;

    UPlayerProfileSaveGame* Profile = Profiles ? Profiles->GetProfile(LocalPlayer) : nullptr;

    if (!Profile || !Profile->Profile.ProfileId.IsValid())
        return;

    const FGuid ProfileId = Profile->Profile.ProfileId;

    if (RunCurrencies.Remove(ProfileId) > 0)
        OnCurrencyChanged.Broadcast(LocalPlayer, ECurrencyStorage::Run, 0);
}

ULocalPlayer* UCurrencySubsystem::GetLocalPlayerFromController(APlayerController* PlayerController)
{
    return PlayerController
        ? PlayerController->GetLocalPlayer()
        : nullptr;
}
void UCurrencySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UPlayerProfileSubsystem* Profiles = Collection.InitializeDependency<UPlayerProfileSubsystem>())
    {
        Profiles->OnProfileAssigned.AddDynamic(this, &UCurrencySubsystem::HandleProfileAssigned);
        Profiles->OnProfileReleased.AddDynamic(this, &UCurrencySubsystem::HandleProfileReleased);
    }
}

void UCurrencySubsystem::Deinitialize()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UPlayerProfileSubsystem* Profiles =
            GI->GetSubsystem<UPlayerProfileSubsystem>())
        {
            Profiles->OnProfileAssigned.RemoveDynamic(this, &UCurrencySubsystem::HandleProfileAssigned);

            Profiles->OnProfileReleased.RemoveDynamic(this, &UCurrencySubsystem::HandleProfileReleased);
        }
    }

    RunCurrencies.Empty();
    ActiveProfileIds.Empty();

    Super::Deinitialize();
}

void UCurrencySubsystem::HandleProfileAssigned(ULocalPlayer* LocalPlayer, FGuid ProfileId)
{
    if (!LocalPlayer || !ProfileId.IsValid())
        return;

    // Remove any old temporary balance for this local player.
    if (const FGuid* OldProfileId =
        ActiveProfileIds.Find(LocalPlayer))
    {
        RunCurrencies.Remove(*OldProfileId);
    }

    // A new profile assignment starts with no run currency.
    RunCurrencies.Remove(ProfileId);
    ActiveProfileIds.Add(LocalPlayer, ProfileId);
}

void UCurrencySubsystem::HandleProfileReleased(ULocalPlayer* LocalPlayer)
{
    if (!LocalPlayer)
        return;

    FGuid ProfileId;

    if (ActiveProfileIds.RemoveAndCopyValue(LocalPlayer, ProfileId))
    {
        RunCurrencies.Remove(ProfileId);
    }
}
