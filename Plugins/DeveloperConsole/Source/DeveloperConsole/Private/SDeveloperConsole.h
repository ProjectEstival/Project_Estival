#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Views/SListView.h"

class FDevConsoleLogSink;
struct FDevConsoleLogEvent;

struct FDevConsoleUIState
{
    TSet<ELogVerbosity::Type> EnabledVerbosity =
    {
        ELogVerbosity::Log,
        ELogVerbosity::Warning,
        ELogVerbosity::Error,
        ELogVerbosity::Fatal,
        // ELogVerbosity::Display,
        // ELogVerbosity::Verbose,
        // ELogVerbosity::VeryVerbose
    };
    float UIScale = 1.0f;
    float ScrollOffset = -1.0f;
};

class SDeveloperConsole : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDeveloperConsole) : _LogSink(nullptr)
    {
    }
        SLATE_ARGUMENT(FDevConsoleLogSink*, LogSink)
        SLATE_ARGUMENT(FDevConsoleUIState, InitialUIState)
        SLATE_EVENT(FSimpleDelegate, OnClearLogs)
    SLATE_END_ARGS()
    
    FDevConsoleUIState GetUIState() const;
    
    void Construct(const FArguments& InArgs);
    void AddEvents(const TArray<FDevConsoleLogEvent>& NewEvents);
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    
private:
    // Events
    FDevConsoleLogSink* LogSink = nullptr;
    TArray<TSharedPtr<FDevConsoleLogEvent>> Events;
    TArray<TSharedPtr<FDevConsoleLogEvent>> VisibleEvents;
    TSharedPtr<SBorder> RootBorder;
    TSharedPtr<SListView<TSharedPtr<FDevConsoleLogEvent>>> ListView;
    TSharedPtr<SMultiLineEditableTextBox> DetailsText;
    TSharedPtr<FDevConsoleLogEvent> SelectedEvent;
    FText GetSelectedDetails() const;
    void UpdateDetails();
    TSharedRef<ITableRow> GenerateRow(TSharedPtr<FDevConsoleLogEvent> Event, const TSharedRef<STableViewBase>& OwnerTable);
    void OnEventSelected(TSharedPtr<FDevConsoleLogEvent> Event, ESelectInfo::Type SelectInfo);
    
    FSimpleDelegate OnClearLogs;
    void ClearLogs();
    
    // Search
    TSharedPtr<SEditableTextBox> SearchBox;
    FString SearchFilter;
    void OnSearchChanged(const FText& NewText);
    void RefreshVisibleEvents();
    bool PassesFilter(const FDevConsoleLogEvent& Event) const;
    
    // Verbosity filters
    TSet<ELogVerbosity::Type> EnabledVerbosity;
    void OnVerbosityToggled(ELogVerbosity::Type Verbosity, ECheckBoxState NewState);
    ECheckBoxState GetVerbosityCheckState(ELogVerbosity::Type Verbosity) const;
    bool bShowCategoryFilters = false;
    TSharedPtr<SWrapBox> CategoryFilterWrap;
    
    // UI scaling
    float Scaled(float Value) const;
    FSlateFontInfo ScaledFont(const TCHAR* Style, int32 BaseSize) const;
    float UIScale = 1.0f;
    static constexpr float MinUIScale = 0.5f;
    static constexpr float MaxUIScale = 2.0f;
    static constexpr float UIScaleStep = 0.05f;
    
    // Scroll persistence
    bool bAfterInitialization = false;
    float ScrollOffset = -1.0f;
};
