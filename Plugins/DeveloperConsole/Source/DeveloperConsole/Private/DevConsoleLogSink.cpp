#include "DevConsoleLogSink.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"
#include "HAL/PlatformTLS.h"
#include "Logging/StructuredLog.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"

namespace
{
    bool ShouldHideConsoleStackFrame(const FString& FunctionName)
    {
        return FunctionName.Contains(TEXT("FDevConsoleLogSink::CaptureStack"), ESearchCase::CaseSensitive)
        || FunctionName.Contains(TEXT("FDevConsoleLogSink::SerializeRecord"), ESearchCase::CaseSensitive);
    }
}

FDevConsoleLogSink::FDevConsoleLogSink() { }
FDevConsoleLogSink::~FDevConsoleLogSink() { }

bool FDevConsoleLogSink::CanBeUsedOnAnyThread() const { return true; }
bool FDevConsoleLogSink::CanBeUsedOnMultipleThreads() const { return true; }

void FDevConsoleLogSink::SerializeRecord(const UE::FLogRecord& Record)
{
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
    
    CaptureStack(Event.Stack);
    
    {
        FScopeLock Lock(&QueueMutex);
        PendingEvents.Add(MoveTemp(Event));
    }
}

void FDevConsoleLogSink::CaptureStack(TArray<FDevConsoleStackFrame>& OutStack)
{
    constexpr uint32 MaxDepth = 64;
    uint64 BackTrace[MaxDepth] = {};
    
    const uint32 FrameCount = FPlatformStackWalk::CaptureStackBackTrace(BackTrace, MaxDepth, nullptr);
    OutStack.Reserve(OutStack.Num() + FrameCount);
    
    for (uint32 Index = 0; Index < FrameCount; ++Index)
    {
        FProgramCounterSymbolInfo Info = {};
        FPlatformStackWalk::ProgramCounterToSymbolInfo(BackTrace[Index], Info);
        
        const FString ModuleName = ANSI_TO_TCHAR(Info.ModuleName);
        const FString FunctionName = ANSI_TO_TCHAR(Info.FunctionName);
        const FString FileName = ANSI_TO_TCHAR(Info.Filename);
        
        // Filter our own logging code from the displayed stack.
        if (ShouldHideConsoleStackFrame(FunctionName)) continue;
        
        FDevConsoleStackFrame& Frame = OutStack.AddDefaulted_GetRef();
        
        Frame.ProgramCounter = BackTrace[Index];
        const FString DisplayModule = ModuleName.IsEmpty() ? TEXT("UnknownModule") : FPaths::GetCleanFilename(ModuleName);
        
        if (!FunctionName.IsEmpty())
        {
            Frame.Symbol = FString::Printf(TEXT("%s!%s"), *DisplayModule, *FunctionName);
            if (Info.LineNumber > 0 && !FileName.IsEmpty()) Frame.Symbol += FString::Printf(TEXT("\n    %s:%d"), *FileName, Info.LineNumber);
        }
        else  Frame.Symbol = FString::Printf(TEXT("%s + 0x%llX"), *DisplayModule, static_cast<unsigned long long>(BackTrace[Index]));
    }
}

void FDevConsoleLogSink::DrainEvents(TArray<FDevConsoleLogEvent>& OutEvents)
{
    FScopeLock Lock(&QueueMutex);
    OutEvents.Append(MoveTemp(PendingEvents));
    PendingEvents.Reset();
}