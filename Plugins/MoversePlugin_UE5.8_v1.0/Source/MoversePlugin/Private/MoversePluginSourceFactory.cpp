// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "MoversePluginSourceFactory.h"

#include "MoversePlugin.h"
#include "SMoversePluginSourceFactory.h"
#include "MoversePluginSource.h"

#define LOCTEXT_NAMESPACE "MoversePluginSourceFactory"

FText UMoversePluginSourceFactory::GetSourceDisplayName() const {
    return LOCTEXT("SourceDisplayName", "Moverse Plugin");
}

FText UMoversePluginSourceFactory::GetSourceTooltip() const {
    return LOCTEXT("SourceTooltip",
        "Creates a connection to Moverse Studio streaming");
}

TSharedPtr<SWidget> UMoversePluginSourceFactory::BuildCreationPanel(
    FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const {
    return SNew(SMoversePluginSourceFactory)
        .OnOkClicked(SMoversePluginSourceFactory::FOnOkClicked::CreateUObject(
            this, &UMoversePluginSourceFactory::OnOkClicked,
            InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> UMoversePluginSourceFactory::CreateSource(
    const FString& InConnectionString) const {
    FIPv4Endpoint DeviceEndPoint;
    if (!FIPv4Endpoint::Parse(InConnectionString, DeviceEndPoint)) {
        return TSharedPtr<ILiveLinkSource>();
    }

    return MakeShared<FMoversePluginSource>(DeviceEndPoint, TEXT("null"), false);
}

void UMoversePluginSourceFactory::OnOkClicked(
    FIPv4Endpoint InEndpoint, FString ActorName, bool LoadAssets,
    FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const {
    InOnLiveLinkSourceCreated.ExecuteIfBound(
        MakeShared<FMoversePluginSource>(InEndpoint, ActorName, LoadAssets),
        InEndpoint.ToString());
}

#undef LOCTEXT_NAMESPACE