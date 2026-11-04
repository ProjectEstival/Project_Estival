
#include "SDeveloperConsole.h"

#include "DevConsoleLogEvent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"

#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SSplitter.h"

#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"

#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SDeveloperConsole"

void SDeveloperConsole::Construct(const FArguments& InArgs)
{
    bAfterInitialization = false;
    
    // Hook to module's event list
    OnClearLogs = InArgs._OnClearLogs;
    LogSink = InArgs._LogSink;
    
    // Restore UI state
    const FDevConsoleUIState& InitialState = InArgs._InitialUIState;
    EnabledVerbosity = InitialState.EnabledVerbosity;
    UIScale = FMath::Clamp(InitialState.UIScale, MinUIScale, MaxUIScale);
    ScrollOffset = InitialState.ScrollOffset;
    
    ChildSlot
    [
        SAssignNew(RootBorder, SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.92f)).Padding(10.0f)
        [
            // Toolbar
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
            [
                SNew(SHorizontalBox)
                // Title
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 12.0f, 0.0f)
                [
                    SNew(STextBlock).Text(LOCTEXT("ConsoleTitle", "Developer Console")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                ]
                // Searchbar
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [
                    SAssignNew(SearchBox, SEditableTextBox).HintText(LOCTEXT("SearchHint", "Filter by category or message...")).OnTextChanged(this, &SDeveloperConsole::OnSearchChanged)
                ]
                
                // Verbosity filters: 
                // Log
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState,ELogVerbosity::Log).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Log, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterLog", "Log"))
                    ]
                ]
                // Warning
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::Warning).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Warning, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterWarning", "Warning"))
                    ]
                ]
                // Error
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::Error).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Error, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterError", "Error"))
                    ]
                ]
                // Fatal
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::Fatal).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Fatal, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterFatal", "Fatal"))
                    ]
                ]
                // Display
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::Display).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Display, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterDisplay", "Display"))
                    ]
                ]
                // Verbose
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::Verbose).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::Verbose, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterVerbose", "Verbose"))
                    ]
                ]
                // Very verbose
                + SHorizontalBox::Slot().AutoWidth().Padding(4.0f).VAlign(VAlign_Center)
                [
                    SNew(SCheckBox).IsChecked(this, &SDeveloperConsole::GetVerbosityCheckState, ELogVerbosity::VeryVerbose).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        OnVerbosityToggled(ELogVerbosity::VeryVerbose, State);
                    })
                    [
                        SNew(STextBlock).Text(LOCTEXT("FilterVeryVerbose", "Very Verbose"))
                    ]
                ]
                
                // Clear logs
                + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SButton).Text(LOCTEXT("ClearLogs", "Clear")).OnClicked_Lambda([this]()
                    {
                        ClearLogs();
                        return FReply::Handled();
                    })
                ]
            ]
            
            // Event viewer & details
            + SVerticalBox::Slot().FillHeight(1.0f).MinHeight(180.0f)
            [
                SNew(SSplitter).Orientation(Orient_Vertical)
                // Event Viewer
                + SSplitter::Slot().Value(0.68f)
                [
                    SNew(SBorder).Padding(4.0f)
                    [
                        SAssignNew(ListView, SListView<TSharedPtr<FDevConsoleLogEvent>>).ListItemsSource(&VisibleEvents).OnGenerateRow(this, &SDeveloperConsole::GenerateRow).OnSelectionChanged(this, &SDeveloperConsole::OnEventSelected).SelectionMode(ESelectionMode::Single)
                    ]
                ]

                // Details (Stack Trace)
                + SSplitter::Slot().Value(0.32f)
                [
                    SNew(SBorder).Padding(8.0f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                        [
                            SNew(STextBlock).Text(LOCTEXT("DetailsHeader", "Selected Log Details")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                        ]
                        + SVerticalBox::Slot().FillHeight(1.0f)
                        [
                            SAssignNew(DetailsText, SMultiLineEditableTextBox).Text(LOCTEXT("DetailsPlaceholder", "Select a log entry to inspect its source and stack trace.")).IsReadOnly(true).AutoWrapText(false).AlwaysShowScrollbars(true)
                            .Font_Lambda([this]()
                            {
                                return ScaledFont(TEXT("Regular"), 10);
                            }).WrapTextAt(0.0f)
                        ]
                    ]
                ]
            ]
        ]
    ];
}

// Event Drawing
TSharedRef<ITableRow> SDeveloperConsole::GenerateRow(TSharedPtr<FDevConsoleLogEvent> Event, const TSharedRef<STableViewBase>& OwnerTable) const
{
    const TCHAR* LevelText = TEXT("LOG");
    FSlateColor LevelColor = FSlateColor(FLinearColor(0.75f, 0.78f, 0.82f));
    
    switch (Event->Verbosity)
    {
        case ELogVerbosity::Warning:
            LevelText = TEXT("WARNING");
            LevelColor = FSlateColor(FLinearColor(1.0f, 0.72f, 0.20f));
            break;
            
        case ELogVerbosity::Error:
        case ELogVerbosity::Fatal:
            LevelText = TEXT("ERROR");
            LevelColor = FSlateColor(FLinearColor(1.0f, 0.32f, 0.32f));
            break;
            
        case ELogVerbosity::Display:
            LevelText = TEXT("DISPLAY");
            break;
            
        case ELogVerbosity::Verbose:
        case ELogVerbosity::VeryVerbose:
            LevelText = TEXT("VERBOSE");
            break;
            
        default:
            break;
    }
    
    const FText TimeText = FText::FromString(Event->Time.ToString(TEXT("%H:%M:%S")));
    
    return SNew(STableRow<TSharedPtr<FDevConsoleLogEvent>>, OwnerTable)
    .Padding_Lambda([this]()
    {
        return FMargin(Scaled(4.0f), Scaled(2.0f));
    })
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(TAttribute<FMargin>::CreateLambda([this]()
        {
            return FMargin(0.0f, 0.0f, Scaled(10.0f),0.0f);
        }))
        [
            SNew(STextBlock).Text(TimeText).ColorAndOpacity(FSlateColor::UseSubduedForeground()).Font_Lambda([this]()
            {
                return ScaledFont(TEXT("Mono"), 9);
            })
        ]
        
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(TAttribute<FMargin>::CreateLambda([this]()
        {
            return FMargin(0.0f, 0.0f, Scaled(10.0f),0.0f);
        }))
        [
            SNew(STextBlock).Text(FText::FromString(LevelText)).ColorAndOpacity(LevelColor).Font_Lambda([this]()
            {
                return ScaledFont(TEXT("Bold"), 9);
            })
        ]
        
        + SHorizontalBox::Slot().AutoWidth().MaxWidth(180.0f).VAlign(VAlign_Center).Padding(TAttribute<FMargin>::CreateLambda([this]()
        {
            return FMargin(0.0f, 0.0f, Scaled(10.0f),0.0f);
        }))
        [
            SNew(STextBlock).Text(FText::FromName(Event->Category)).ColorAndOpacity(FSlateColor::UseSubduedForeground()).Font_Lambda([this]()
            {
                return ScaledFont(TEXT("Regular"), 9);
            })
        ]
        
        + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
        [
            SNew(STextBlock).Text(FText::FromString(Event->Message)).ToolTipText(FText::FromString(Event->Message)).Font_Lambda([this]()
            {
                return ScaledFont(TEXT("Regular"), 10);
            }).WrapTextAt(0.0f)
        ]
    ];
}
void SDeveloperConsole::AddEvents(const TArray<FDevConsoleLogEvent>& NewEvents)
{
    if (NewEvents.IsEmpty()) return;
    const bool wasNearBottom = ListView->GetScrollDistanceRemaining().Y <= 0.01f;
    for (const FDevConsoleLogEvent& Event : NewEvents)
    {
        Events.Add(MakeShared<FDevConsoleLogEvent>(Event));
    }
    RefreshVisibleEvents();
    if (bAfterInitialization)
    {
        if (wasNearBottom) ListView->ScrollToBottom();
    }
    else
    {
        if (ScrollOffset >= 0.0f) ListView->SetScrollOffset(ScrollOffset);
        else ListView->ScrollToBottom();
    }
    bAfterInitialization = true;
}

// Event Selection
void SDeveloperConsole::OnEventSelected(TSharedPtr<FDevConsoleLogEvent> Event, ESelectInfo::Type SelectInfo)
{
    SelectedEvent = Event;
    UpdateDetails();
}

// UI Scaling (Ctrl+Scroll)
FReply SDeveloperConsole::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (!MouseEvent.IsControlDown()) return FReply::Unhandled();
    const float OldScale = UIScale;
    UIScale = FMath::Clamp(UIScale + MouseEvent.GetWheelDelta() * UIScaleStep, MinUIScale, MaxUIScale);
    if (!FMath::IsNearlyEqual(OldScale, UIScale))
    {
        Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
        if (ListView.IsValid()) ListView->RequestListRefresh();
    }
    return FReply::Handled();
}
float SDeveloperConsole::Scaled(float Value) const
{
    return Value * UIScale;
}
FSlateFontInfo SDeveloperConsole::ScaledFont(const TCHAR* Style, int32 BaseSize) const
{
    FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(Style, BaseSize);
    Font.Size = FMath::Max(1, FMath::RoundToInt(BaseSize * UIScale));
    return Font;
}

// Block Clicks
FReply SDeveloperConsole::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return FReply::Handled();
}
FReply SDeveloperConsole::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return FReply::Handled();
}

// Search & Filtering
void SDeveloperConsole::RefreshVisibleEvents()
{
    VisibleEvents.Reset();
    for (const TSharedPtr<FDevConsoleLogEvent>& Event : Events)
    {
        if (Event.IsValid() && PassesFilter(*Event)) VisibleEvents.Add(Event);
    }
    
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
        
        if (SelectedEvent.IsValid() && VisibleEvents.Contains(SelectedEvent)) ListView->SetSelection(SelectedEvent);
        else
        {
            SelectedEvent.Reset();
            ListView->ClearSelection();
        }
    }
    UpdateDetails();
}
bool SDeveloperConsole::PassesFilter(const FDevConsoleLogEvent& Event) const
{
    if (!SearchFilter.IsEmpty())
    {
        const bool bMatchesMessage = Event.Message.Contains(SearchFilter, ESearchCase::IgnoreCase);
        const bool bMatchesCategory = Event.Category.ToString().Contains(SearchFilter, ESearchCase::IgnoreCase);

        if (!bMatchesMessage && !bMatchesCategory) return false;
    }
    if (!EnabledVerbosity.Contains(Event.Verbosity)) return false;
    return true;
}
void SDeveloperConsole::OnSearchChanged(const FText& NewText)
{
    SearchFilter = NewText.ToString();
    RefreshVisibleEvents();
}
void SDeveloperConsole::OnVerbosityToggled(ELogVerbosity::Type Verbosity, ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked) EnabledVerbosity.Add(Verbosity);
    else EnabledVerbosity.Remove(Verbosity);
    RefreshVisibleEvents();
}
ECheckBoxState SDeveloperConsole::GetVerbosityCheckState(ELogVerbosity::Type Verbosity) const
{
    return EnabledVerbosity.Contains(Verbosity) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SDeveloperConsole::ClearLogs()
{
    OnClearLogs.ExecuteIfBound();
    
    Events.Reset();
    VisibleEvents.Reset();
    SelectedEvent.Reset();
    
    if (ListView.IsValid())
    {
        ListView->ClearSelection();
        ListView->RequestListRefresh();
    }
    
    UpdateDetails();
}

// Details Panel (Stack Trace)
void SDeveloperConsole::UpdateDetails() const
{
    if (DetailsText.IsValid()) DetailsText->SetText(GetSelectedDetails());
}
FText SDeveloperConsole::GetSelectedDetails() const
{
    if (!SelectedEvent.IsValid()) return LOCTEXT("NoSelection", "Select a log entry to inspect its source and stack trace.");
    
    const FDevConsoleLogEvent& Event = *SelectedEvent;
    
    FString Details;
    Details += Event.Message + TEXT("\n\n");
    Details += FString::Printf(TEXT("Category: %s\nThread ID: %u\n"), *Event.Category.ToString(), Event.ThreadId);
    
    if (!Event.SourceFile.IsEmpty()) Details += FString::Printf(TEXT("Source: %s:%d\n"), *Event.SourceFile, Event.SourceLine);
    
    Details += TEXT("\nStack:\n");
    
    if (Event.Stack.IsEmpty()) Details += TEXT("No stack captured for this event.\n");
    else
    {
        for (const FDevConsoleStackFrame& Frame : Event.Stack)
        {
            Details += TEXT("  ");
            Details += Frame.Symbol;
            Details += TEXT("\n");
        }
    }
    
    return FText::FromString(Details);
}

// UI State
FDevConsoleUIState SDeveloperConsole::GetUIState() const
{
    FDevConsoleUIState State;
    State.EnabledVerbosity = EnabledVerbosity;
    State.UIScale = UIScale;
    State.ScrollOffset = ListView->GetScrollDistanceRemaining().Y <= 0.01f ? -1.0f : ListView->GetScrollOffset();
    return State;
}
#undef LOCTEXT_NAMESPACE
