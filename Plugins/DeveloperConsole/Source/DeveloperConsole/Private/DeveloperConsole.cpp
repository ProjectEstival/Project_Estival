#include "DeveloperConsole.h"
#include "DevConsoleLogSink.h"
#include "SDeveloperConsole.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

namespace
{
    class FDevConsoleInputProcessor final : public IInputProcessor
    {
    public:
        explicit FDevConsoleInputProcessor(TFunction<void()> InToggle) : Toggle(MoveTemp(InToggle)) { }
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
        virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override
        {
            if (Event.GetKey() == EKeys::F10 && !Event.IsRepeat())
            {
                if (Toggle) Toggle();
                return true;
            }
            
            return false;
        }
        
        virtual const TCHAR* GetDebugName() const override
        {
            return TEXT("DevConsoleInputProcessor");
        }
    
    private:
        TFunction<void()> Toggle;
    };
}

#define LOCTEXT_NAMESPACE "DeveloperConsole"

void FDeveloperConsoleModule::StartupModule()
{
#if WITH_DEVELOPER_CONSOLE
    FPlatformStackWalk::InitStackWalking();
    
    LogSink = MakeUnique<FDevConsoleLogSink>();
    if (GLog) GLog->AddOutputDevice(LogSink.Get());
    
    RegisterConsoleCommands();
    TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FDeveloperConsoleModule::Tick), 0.0f);
    
    if (FSlateApplication::IsInitialized())
    {
        InputProcessor = MakeShared<FDevConsoleInputProcessor>([this]() { ToggleConsole(); });
        FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor.ToSharedRef());
    }
#endif
#if WITH_EDITOR
    if (GIsEditor) EndPIEDelegateHandle = FEditorDelegates::EndPIE.AddRaw(this, &FDeveloperConsoleModule::OnPIESessionEnded);
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
    
    if (InputProcessor.IsValid() && FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor.ToSharedRef());
    InputProcessor.Reset();
#endif
#if WITH_EDITOR
    if (GIsEditor && EndPIEDelegateHandle.IsValid()) FEditorDelegates::EndPIE.Remove(EndPIEDelegateHandle);
#endif
}
bool FDeveloperConsoleModule::Tick(float DeltaTime)
{
#if WITH_DEVELOPER_CONSOLE
    if (LogSink)
    {
        TArray<FDevConsoleLogEvent> NewEvents;
        LogSink->DrainEvents(NewEvents);
        
        if (NewEvents.Num() > 0)
        {
            // Keep a copy for the visible UI.
            TArray<FDevConsoleLogEvent> EventsForWidget = NewEvents;
            for (FDevConsoleLogEvent& Event : NewEvents)
            {
                RetainedEvents.Add(MoveTemp(Event));
            }
            
            if (ConsoleWidget.IsValid()) ConsoleWidget->AddEvents(EventsForWidget);
        }
        const int32 MaxEvents = MaxEventsVariable ? FMath::Max(1, MaxEventsVariable->GetInt()) : MaxEventsFallback;
        if (RetainedEvents.Num() > MaxEvents) RetainedEvents.RemoveAt(0, RetainedEvents.Num() - MaxEvents, EAllowShrinking::No);
    }
#endif
    return true;
}
#if WITH_EDITOR
void FDeveloperConsoleModule::OnPIESessionEnded(bool bIsSimulating)
{
    bConsoleVisible = false;
}
#endif

void FDeveloperConsoleModule::RegisterConsoleCommands()
{
#if WITH_DEVELOPER_CONSOLE
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    MaxEventsVariable = ConsoleManager.RegisterConsoleVariable(TEXT("devconsole.MaxEvents"), MaxEventsFallback, TEXT("Maximum number of log events retained by the developer console."), ECVF_Default);
#endif
}
void FDeveloperConsoleModule::UnregisterConsoleCommands()
{
#if WITH_DEVELOPER_CONSOLE
    if (MaxEventsVariable)
    {
        IConsoleManager::Get().UnregisterConsoleObject(MaxEventsVariable, false);
        MaxEventsVariable = nullptr;
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
    
    ConsoleWidget = SNew(SDeveloperConsole).LogSink(LogSink.Get()).OnClearLogs(FSimpleDelegate::CreateRaw(this, &FDeveloperConsoleModule::ClearRetainedLogs)).InitialUIState(SavedUIState);
    
    // Populate the new widget with history collected before it opened.
    ConsoleWidget->AddEvents(RetainedEvents);
    
    GEngine->GameViewport->AddViewportWidgetContent(ConsoleWidget.ToSharedRef(), 10000);
    bConsoleVisible = true;
#endif
}
void FDeveloperConsoleModule::HideConsole()
{
#if WITH_DEVELOPER_CONSOLE
    if (!bConsoleVisible) return;
    if (ConsoleWidget.IsValid())
    {
        SavedUIState = ConsoleWidget->GetUIState();
        if (GEngine && GEngine->GameViewport && ConsoleWidget.IsValid()) GEngine->GameViewport->RemoveViewportWidgetContent(ConsoleWidget.ToSharedRef());
        ConsoleWidget.Reset();
    }
    bConsoleVisible = false;
#endif
}

void FDeveloperConsoleModule::ClearRetainedLogs()
{
    RetainedEvents.Reset();
}
#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FDeveloperConsoleModule, DeveloperConsole)