#pragma once

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

    void ShowConsole();
    void HideConsole();

    bool Tick(float DeltaTime);

    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();

private:

    TUniquePtr<FDevConsoleLogSink> LogSink;

    TSharedPtr<SDeveloperConsole> ConsoleWidget;

    FTSTicker::FDelegateHandle TickHandle;

    IConsoleCommand* ToggleCommand = nullptr;

    bool bConsoleVisible = false;
};