// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SMoversePluginSourceFactory.h"

#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MoversePluginSourceEditor"

void SMoversePluginSourceFactory::Construct(const FArguments& Args) {
  OkClicked = Args._OnOkClicked;

  FIPv4Endpoint Endpoint;
  Endpoint.Address = FIPv4Address::Any;
  Endpoint.Port = 54321;

  ChildSlot[SNew(SBox).WidthOverride(
      250)[SNew(SVerticalBox) +
           SVerticalBox::Slot().AutoHeight()
               [SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Left)
                    .FillWidth(0.5f)[SNew(STextBlock)
                                         .Text(LOCTEXT("StreamPortNumber",
                                                       "IP address : PORT"))] +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .FillWidth(
                        0.5f)[SAssignNew(EditabledText, SEditableTextBox)
                                  .Text(FText::FromString(Endpoint.ToString()))
                                  .OnTextCommitted(
                                      this, &SMoversePluginSourceFactory::
                                                OnEndpointChanged)]] +
           SVerticalBox::Slot().AutoHeight()
               [SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Left)
                    .FillWidth(
                        0.5f)[SNew(STextBlock)
                                  .Text(LOCTEXT("ActorName", "Actor Name"))] +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .FillWidth(
                        0.5f)[SAssignNew(EditabledTextActor, SEditableTextBox)
                                  .Text(ActorName)
                                  .OnTextCommitted(
                                      this, &SMoversePluginSourceFactory::
                                                OnActorNameChanged)]] +

           SVerticalBox::Slot().AutoHeight()
               [SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Left)
                    .FillWidth(
                        0.5f)[SNew(STextBlock)
                                  .Text(LOCTEXT("Actor", "Import Assets"))] +
                SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .FillWidth(
                        0.5f)[SAssignNew(EditableCheckbox, SCheckBox)
                                  .OnCheckStateChanged(
                                      this, &SMoversePluginSourceFactory::
                                                HandleCheckBoxStateChanged)]] +
           SVerticalBox::Slot()
               .HAlign(HAlign_Right)
               .AutoHeight()
                   [SNew(SButton).OnClicked(
                       this, &SMoversePluginSourceFactory::OnOkClicked)
                        [SNew(STextBlock).Text(LOCTEXT("Ok", "Ok"))]]]];
}

void SMoversePluginSourceFactory::OnEndpointChanged(const FText& NewValue,
                                                    ETextCommit::Type) {
  TSharedPtr<SEditableTextBox> EditabledTextPin = EditabledText.Pin();
  if (EditabledTextPin.IsValid()) {
    FIPv4Endpoint Endpoint;
    if (!FIPv4Endpoint::Parse(NewValue.ToString(), Endpoint)) {
      Endpoint.Address = FIPv4Address::Any;
      Endpoint.Port = 54321;
      EditabledTextPin->SetText(FText::FromString(Endpoint.ToString()));
    }
  }
}

void SMoversePluginSourceFactory::OnActorNameChanged(const FText& NewValue,
                                                     ETextCommit::Type) {
  ActorName = NewValue;
}

void SMoversePluginSourceFactory::HandleCheckBoxStateChanged(
    ECheckBoxState NewState) {
  ImportAssets = NewState == ECheckBoxState::Checked;
}

FReply SMoversePluginSourceFactory::OnOkClicked() {
  TSharedPtr<SEditableTextBox> EditabledTextPin = EditabledText.Pin();
  if (EditabledTextPin.IsValid()) {
    FIPv4Endpoint Endpoint;
    if (FIPv4Endpoint::Parse(EditabledTextPin->GetText().ToString(),
                             Endpoint)) {
      OkClicked.ExecuteIfBound(Endpoint, ActorName.ToString(), ImportAssets);
    }
  }
  return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE