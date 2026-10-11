#include "DevConsoleLogSink.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"
#include "HAL/PlatformTLS.h"
#include "Logging/StructuredLog.h"
#include "Misc/StringBuilder.h"

FDevConsoleLogSink::FDevConsoleLogSink()
{
}

FDevConsoleLogSink::~FDevConsoleLogSink()
{
}

bool FDevConsoleLogSink::CanBeUsedOnAnyThread() const
{
    return true;
}

bool FDevConsoleLogSink::CanBeUsedOnMultipleThreads() const
{
    return true;
}

void FDevConsoleLogSink::SerializeRecord(const UE::FLogRecord& Record)
{
    // Must be thread safe and also don't call GLog to avoid infinite loops
    
    FDevConsoleLogEvent Event;

    Event.Id = NextEventId.fetch_add(1, std::memory_order_relaxed);
    Event.Time = Record.GetTime().GetUtcTime();
    Event.Category = Record.GetCategory();
    Event.Verbosity = Record.GetVerbosity();
    Event.SourceLine = Record.GetLine();

    if (const ANSICHAR* File = Record.GetFile()) Event.SourceFile = UTF8_TO_TCHAR(File);

    Event.ThreadId = FPlatformTLS::GetCurrentThreadId();

    // FLogRecord stores the original format plus its structured arguments.
    // FormatMessageTo gives us the actual formatted message.
    TStringBuilderWithBuffer<TCHAR, 4096> MessageBuilder;
    Record.FormatMessageTo(MessageBuilder);
    Event.Message = FString(MessageBuilder);

    // Only capture stacks for warnings and errors.
    // Verbosity is an enum with flags, so mask those flags away before comparing the actual verbosity level.
    const ELogVerbosity::Type Verbosity = static_cast<ELogVerbosity::Type>(static_cast<uint8>(Record.GetVerbosity()) & ELogVerbosity::VerbosityMask);

    if (Verbosity == ELogVerbosity::Warning || Verbosity == ELogVerbosity::Error) CaptureStack(Event.Stack);

    {
        FScopeLock Lock(&QueueMutex);
        PendingEvents.Add(MoveTemp(Event));
    }
}

void FDevConsoleLogSink::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
    FDevConsoleLogEvent Event;

    Event.Id = NextEventId.fetch_add(1);
    Event.Time = FDateTime::UtcNow();
    Event.Category = Category;
    Event.Verbosity = static_cast<ELogVerbosity::Type>(static_cast<uint8>(Verbosity) & ELogVerbosity::VerbosityMask);
    Event.Message = Message ? Message : TEXT("");
    Event.ThreadId = FPlatformTLS::GetCurrentThreadId();

    if (Event.Verbosity == ELogVerbosity::Warning || Event.Verbosity == ELogVerbosity::Error) CaptureStack(Event.Stack);

    {
        FScopeLock Lock(&QueueMutex);
        PendingEvents.Add(MoveTemp(Event));
    }
}

void FDevConsoleLogSink::CaptureStack(TArray<FDevConsoleStackFrame>& OutStack) const
{
    constexpr uint32 MaxDepth = 64;
    uint64 BackTrace[MaxDepth];
    
    const uint32 FrameCount = FPlatformStackWalk::CaptureStackBackTrace(BackTrace, MaxDepth, nullptr);
    OutStack.Reserve(FrameCount);

    for (uint32 Index = 0; Index < FrameCount; ++Index)
    {
        FDevConsoleStackFrame& Frame = OutStack.AddDefaulted_GetRef();
        Frame.ProgramCounter = BackTrace[Index];

        // Resolve the address to a human-readable symbol.
        // Right now, StackWalkAndDump/ProgramCounterToSymbolInfo gives us the engine's existing symbolization machinery.
        FProgramCounterSymbolInfo SymbolInfo;
        FPlatformStackWalk::ProgramCounterToSymbolInfo(BackTrace[Index], SymbolInfo);

        if (SymbolInfo.FunctionName[0] != '\0')
        {
            const TCHAR* ModuleName = ANSI_TO_TCHAR(SymbolInfo.ModuleName);
            const TCHAR* FunctionName = ANSI_TO_TCHAR(SymbolInfo.FunctionName);
            Frame.Symbol = FString::Printf(TEXT("%s!%s"), ModuleName, FunctionName);
        }
        else Frame.Symbol = FString::Printf(TEXT("0x%llX"), static_cast<unsigned long long>(BackTrace[Index]));
    }
}

void FDevConsoleLogSink::DrainEvents(TArray<FDevConsoleLogEvent>& OutEvents)
{
    FScopeLock Lock(&QueueMutex);
    OutEvents.Append(MoveTemp(PendingEvents));
    PendingEvents.Reset();
}