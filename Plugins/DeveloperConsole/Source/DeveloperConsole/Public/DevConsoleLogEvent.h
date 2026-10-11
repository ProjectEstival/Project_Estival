#pragma once

#include "CoreMinimal.h"

struct FDevConsoleStackFrame
{
    uint64 ProgramCounter = 0;
    FString Symbol;
};

struct FDevConsoleLogEvent
{
    uint64 Id = 0;

    FDateTime Time;

    FName Category = NAME_None;
    ELogVerbosity::Type Verbosity = ELogVerbosity::Log;

    FString Message;

    FString SourceFile;
    int32 SourceLine = 0;

    uint32 ThreadId = 0;

    TArray<FDevConsoleStackFrame> Stack;
};