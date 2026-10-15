#pragma once

#include "DevConsoleLogEvent.h"
#include "SDeveloperConsole.h"
#include "Modules/ModuleManager.h"

class FDevConsoleLogSink;
class SDeveloperConsole;

class FDeveloperConsoleModule : public IModuleInterface
{
public:
    
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void ToggleConsole();
    
private:
    FTSTicker::FDelegateHandle TickHandle;    
    bool Tick(float DeltaTime);
    
    // Events list
    TArray<FDevConsoleLogEvent> RetainedEvents;
    void ClearRetainedLogs();
    
    IConsoleVariable* MaxEventsVariable = nullptr;
    int32 MaxEventsFallback = 5000;
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    
    // Other module components
    TUniquePtr<FDevConsoleLogSink> LogSink;
    TSharedPtr<SDeveloperConsole> ConsoleWidget;
    
    // Toggling
    void ShowConsole();
    void HideConsole();
    TSharedPtr<IInputProcessor> InputProcessor;
    bool bConsoleVisible = false;
    
    FDevConsoleUIState SavedUIState;
};