// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

class SEditableTextBox;

class SMoversePluginSourceFactory : public SCompoundWidget {
public:
    DECLARE_DELEGATE_ThreeParams(FOnOkClicked, FIPv4Endpoint, FString, bool);

    SLATE_BEGIN_ARGS(SMoversePluginSourceFactory) {}
        SLATE_EVENT(FOnOkClicked, OnOkClicked)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    void OnEndpointChanged(const FText& NewValue, ETextCommit::Type);
    void OnActorNameChanged(const FText& NewValue, ETextCommit::Type);
    void HandleCheckBoxStateChanged(ECheckBoxState NewState);

    FReply OnOkClicked();

    TWeakPtr<SEditableTextBox> EditabledText, EditabledTextActor;
    TWeakPtr<SCheckBox> EditableCheckbox;
    FOnOkClicked OkClicked;
    FText ActorName;
    bool ImportAssets{ false };
};