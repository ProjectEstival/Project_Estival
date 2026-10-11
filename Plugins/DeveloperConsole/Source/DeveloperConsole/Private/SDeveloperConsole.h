#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FDevConsoleLogSink;
struct FDevConsoleLogEvent;

class SDeveloperConsole : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SDeveloperConsole)
        : _LogSink(nullptr)
    {
    }
    SLATE_ARGUMENT(FDevConsoleLogSink*, LogSink)
    SLATE_END_ARGS()

    void Construct(
        const FArguments& InArgs
    );

    void AddEvents(
        TArray<FDevConsoleLogEvent>& Events
    );

private:

    TSharedRef<ITableRow> GenerateRow(
        TSharedPtr<FDevConsoleLogEvent> Event,
        const TSharedRef<STableViewBase>& OwnerTable
    );

    void OnEventSelected(
        TSharedPtr<FDevConsoleLogEvent> Event,
        ESelectInfo::Type SelectInfo
    );

private:

    FDevConsoleLogSink* LogSink = nullptr;

    TArray<TSharedPtr<FDevConsoleLogEvent>> Events;

    TSharedPtr<SListView<TSharedPtr<FDevConsoleLogEvent>>> ListView;

    TSharedPtr<STextBlock> DetailsText;
};