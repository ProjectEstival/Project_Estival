#include "DeveloperConsole.h"

#include "DevConsoleLogSink.h"
#include "SDeveloperConsole.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "DeveloperConsole"

void FDeveloperConsoleModule::StartupModule()
{
#if WITH_DEVELOPER_CONSOLE

    LogSink = MakeUnique<FDevConsoleLogSink>();
    if (GLog) GLog->AddOutputDevice(LogSink.Get());

    RegisterConsoleCommands();
    TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FDeveloperConsoleModule::Tick), 0.0f);

#endif
}

void FDeveloperConsoleModule::ShutdownModule()
{
#if WITH_DEVELOPER_CONSOLE

    if (TickHandle.IsValid())
    {
        FTSTicker::RemoveTicker(TickHandle);
        TickHandle.Reset();
    }

    HideConsole();
    UnregisterConsoleCommands();
    if (GLog && LogSink.IsValid()) GLog->RemoveOutputDevice(LogSink.Get());
    LogSink.Reset();

#endif
}

void FDeveloperConsoleModule::RegisterConsoleCommands()
{
#if WITH_DEVELOPER_CONSOLE

    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    ToggleCommand = ConsoleManager.RegisterConsoleCommand(
        TEXT("devconsole.toggle"),
        TEXT("Toggle the developer console."),
        FConsoleCommandDelegate::CreateRaw(this, &FDeveloperConsoleModule::ToggleConsole),
        ECVF_Default
    );
    
#endif
}

void FDeveloperConsoleModule::UnregisterConsoleCommands()
{
#if WITH_DEVELOPER_CONSOLE
    if (ToggleCommand)
    {
        IConsoleManager::Get().UnregisterConsoleObject(ToggleCommand, false);
        ToggleCommand = nullptr;
    }
#endif
}

void FDeveloperConsoleModule::ToggleConsole()
{
#if WITH_DEVELOPER_CONSOLE
    if (bConsoleVisible) HideConsole();
    else ShowConsole();
#endif
}

void FDeveloperConsoleModule::ShowConsole()
{
#if WITH_DEVELOPER_CONSOLE
    if (bConsoleVisible) return;
    
    if (!GEngine || !GEngine->GameViewport)
    {
        UE_LOG(LogTemp, Warning, TEXT("Developer console could not find a game viewport."));
        return;
    }
    ConsoleWidget = SNew(SDeveloperConsole).LogSink(LogSink.Get());
    GEngine->GameViewport->AddViewportWidgetContent(ConsoleWidget.ToSharedRef(), 10000);
    bConsoleVisible = true;
    
#endif
}

void FDeveloperConsoleModule::HideConsole()
{
#if WITH_DEVELOPER_CONSOLE
    if (!bConsoleVisible) return;
    
    if (GEngine && GEngine->GameViewport && ConsoleWidget.IsValid()) GEngine->GameViewport->RemoveViewportWidgetContent(ConsoleWidget.ToSharedRef());
    ConsoleWidget.Reset();
    bConsoleVisible = false;
    
#endif
}

bool FDeveloperConsoleModule::Tick(float DeltaTime)
{
#if WITH_DEVELOPER_CONSOLE
    if (LogSink)
    {
        TArray<FDevConsoleLogEvent> NewEvents;
        LogSink->DrainEvents(NewEvents);
        if (NewEvents.Num() > 0 && ConsoleWidget.IsValid()) ConsoleWidget->AddEvents(NewEvents);
    }
#endif
    return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDeveloperConsoleModule, DeveloperConsole)