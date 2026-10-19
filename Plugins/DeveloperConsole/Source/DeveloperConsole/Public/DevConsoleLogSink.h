#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

#include "DevConsoleLogEvent.h"

class FDevConsoleLogSink : public FOutputDevice
{
public:
    FDevConsoleLogSink();
    
    virtual ~FDevConsoleLogSink() override;
    
    virtual bool CanBeUsedOnAnyThread() const override;
    virtual bool CanBeUsedOnMultipleThreads() const override;
    virtual void SerializeRecord(const UE::FLogRecord& Record) override;
    virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category) override;
    
    void DrainEvents(TArray<FDevConsoleLogEvent>& OutEvents);
    
private:
    static void CaptureStack(TArray<FDevConsoleStackFrame>& OutStack);
    
    FCriticalSection QueueMutex;
    
    TArray<FDevConsoleLogEvent> PendingEvents;
    
    std::atomic<uint64> NextEventId { 1 };
};