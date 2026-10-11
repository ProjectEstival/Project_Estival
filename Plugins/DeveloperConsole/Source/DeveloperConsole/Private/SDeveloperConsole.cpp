#include "SDeveloperConsole.h"

#include "DevConsoleLogEvent.h"
#include "DevConsoleLogSink.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

void SDeveloperConsole::Construct(const FArguments& InArgs)
{
    LogSink = InArgs._LogSink;

    ChildSlot
    [
        SNew(SBorder).Padding(8.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SAssignNew(ListView, SListView<TSharedPtr<FDevConsoleLogEvent>>)
                .ListItemsSource(&Events)
                .OnGenerateRow(this, &SDeveloperConsole::GenerateRow)
                .OnSelectionChanged(this, &SDeveloperConsole::OnEventSelected)
                .SelectionMode(ESelectionMode::Single)
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [SAssignNew(DetailsText, STextBlock).Text(FText::FromString(TEXT("Select a warning or error to inspect it."))).AutoWrapText(true)]
        ]
    ];
}

void SDeveloperConsole::AddEvents(
    TArray<FDevConsoleLogEvent>& NewEvents)
{
    for (FDevConsoleLogEvent& Event : NewEvents)
    {
        Events.Add(MakeShared<FDevConsoleLogEvent>(MoveTemp(Event)));
    }

    // Hardcoded bound.
    constexpr int32 MaxEvents = 2000;

    if (Events.Num() > MaxEvents)
    {
        const int32 RemoveCount = Events.Num() - MaxEvents;

        Events.RemoveAt(0, RemoveCount, EAllowShrinking::No);
    }

    if (ListView.IsValid()) ListView->RequestListRefresh();
}

TSharedRef<ITableRow> SDeveloperConsole::GenerateRow(TSharedPtr<FDevConsoleLogEvent> Event, const TSharedRef<STableViewBase>& OwnerTable)
{
    FString VerbosityText;

    switch (Event->Verbosity)
    {
        case ELogVerbosity::Warning:
            VerbosityText = TEXT("Warning");
            break;
        case ELogVerbosity::Error:
            VerbosityText = TEXT("Error");
            break;
        case ELogVerbosity::Fatal:
            VerbosityText = TEXT("Fatal");
            break;
        default:
            VerbosityText = TEXT("Log");
            break;
    }

    const FString TimeText = Event->Time.ToString(TEXT("%H:%M:%S.%s"));
    const FString RowText = FString::Printf(TEXT("%s  %-8s  %-24s  %s"), *TimeText, *VerbosityText, *Event->Category.ToString(), *Event->Message);
    return SNew(STableRow<TSharedPtr<FDevConsoleLogEvent>>, OwnerTable)[SNew(STextBlock).Text(FText::FromString(RowText)).ToolTipText(FText::FromString(Event->Message))];
}

void SDeveloperConsole::OnEventSelected(TSharedPtr<FDevConsoleLogEvent> Event, ESelectInfo::Type SelectInfo)
{
    if (!Event.IsValid()) return;

    FString Details;

    Details += FString::Printf(TEXT("%s\n\n"), *Event->Message);
    Details += FString::Printf(TEXT("Category: %s\n"), *Event->Category.ToString());
    Details += FString::Printf(TEXT("Thread ID: %u\n"), Event->ThreadId);

    if (!Event->SourceFile.IsEmpty()) Details += FString::Printf(TEXT("Source: %s:%d\n"), *Event->SourceFile, Event->SourceLine);

    if (Event->Stack.Num() > 0)
    {
        Details += TEXT("\nStack:\n");

        for (const FDevConsoleStackFrame& Frame : Event->Stack)
        {
            Details += FString::Printf(TEXT("  %s\n"), *Frame.Symbol);
        }
    }

    if (DetailsText.IsValid()) DetailsText->SetText(FText::FromString(Details));
}