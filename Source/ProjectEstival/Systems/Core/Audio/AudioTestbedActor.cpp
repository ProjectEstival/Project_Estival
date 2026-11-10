#include "Systems/Core/Audio/AudioTestbedActor.h"

#include "FMODEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"

AAudioTestbedActor::AAudioTestbedActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AudioRoot"));
}

void AAudioTestbedActor::BeginPlay()
{
    Super::BeginPlay();

    AudioManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAudioManager>() : nullptr;

    SetActorTickEnabled(bEnableTickTest && IsValid(AudioManager));
    PrintTest(IsValid(AudioManager) ? TEXT("Testbed initialized.") : TEXT("FAILED: AudioManager unavailable."));
}

void AAudioTestbedActor::PrintTest(const FString& Message) const
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[AudioTest] %s"), *Message));
}

void AAudioTestbedActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsValid(AudioManager)) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    const auto Pressed = [PC](FKey Key) { return PC->WasInputKeyJustPressed(Key); };

    const auto EnsureLoop = [this](TObjectPtr<UFMODAudioComponent>& Slot)
    {
        if (!IsValid(LoopEvent)) return false;

        if (!IsValid(Slot))
            Slot = AudioManager->CreateAttachedAudio(LoopEvent, GetRootComponent());

        if (!IsValid(Slot)) return false;

        if (!Slot->IsPlaying()) Slot->Play();
        return true;
    };

    const auto DestroyLoop = [this](TObjectPtr<UFMODAudioComponent>& Slot)
    {
        if (!IsValid(Slot)) return;

        AudioManager->DestroyAudio(Slot);
        Slot = nullptr;
    };

    const auto SetMusicParameter = [this](float Value, float Duration)
    {
        UFMODAudioComponent* Music = AudioManager->GetMusicComponent();

        if (!IsValid(Music) || (!AudioManager->IsMusicPlaying() && !AudioManager->IsMusicPaused()))
        {
            PrintTest(TEXT("Music parameter: No active music."));
            return;
        }

        const bool bSuccess = AudioManager->SetInstanceParameter(Music, MusicParameterName, Value, Duration);
        PrintTest(FString::Printf(TEXT("Music %s => %.1f (%.1fs): %s"), *MusicParameterName.ToString(), Value, Duration, bSuccess ? TEXT("OK") : TEXT("FAILED")));
    };

    const auto SetGlobal = [this](float Value, float Duration)
    {
        const bool bSuccess = AudioManager->SetGlobalParameter(GlobalParameterName, Value, Duration);
        PrintTest(FString::Printf(TEXT("Global %s => %.1f (%.1fs): %s"), *GlobalParameterName.ToString(), Value, Duration, bSuccess ? TEXT("OK") : TEXT("FAILED")));
    };

    const auto ToggleVolume = [this](const TCHAR* BusName, float Current, auto Setter)
    {
        if (Current < 0.f)
        {
            PrintTest(FString::Printf(TEXT("%s Bus: FAILED to read volume."), BusName));
            return;
        }

        const float Target = Current > 0.99f ? 0.25f : 1.f;
        const bool bSuccess = Setter(Target);

        PrintTest(FString::Printf(TEXT("%s Bus => %.0f%%: %s"), BusName, Target * 100.f, bSuccess ? TEXT("OK") : TEXT("FAILED")));
    };

    // -----------------------------------------
    // MUSIC PLAYBACK
    // -----------------------------------------

    if (Pressed(EKeys::One))
    {
        const bool bSuccess = AudioManager->PlayMusic(TestMusicA);
        PrintTest(bSuccess ? TEXT("Playing Music A.") : TEXT("FAILED: Music A."));
    }

    if (Pressed(EKeys::Two))
    {
        const bool bSuccess = AudioManager->PlayMusic(TestMusicB);
        PrintTest(bSuccess ? TEXT("Playing Music B.") : TEXT("FAILED: Music B."));
    }

    if (Pressed(EKeys::Three))
    {
        AudioManager->StopMusic();
        PrintTest(TEXT("Music stopped."));
    }

    if (Pressed(EKeys::Four))
    {
        const bool bPaused = !AudioManager->IsMusicPaused();
        AudioManager->SetMusicPaused(bPaused);
        PrintTest(AudioManager->IsMusicPaused() ? TEXT("Music paused.") : TEXT("Music resumed / not playing."));
    }

    // -----------------------------------------
    // 2D AMBIENCE
    // -----------------------------------------

    if (Pressed(EKeys::Five))
    {
        if (!IsValid(AmbienceComponent))
            AmbienceComponent = AudioManager->Create2DAudio(AmbienceEvent, this);

        if (IsValid(AmbienceComponent))
        {
            if (!AmbienceComponent->IsPlaying()) AmbienceComponent->Play();
            PrintTest(TEXT("2D ambience playing."));
        }
        else
        {
            PrintTest(TEXT("FAILED: Create2DAudio."));
        }
    }

    if (Pressed(EKeys::Six))
    {
        AudioManager->DestroyAudio(AmbienceComponent);
        AmbienceComponent = nullptr;
        PrintTest(TEXT("2D ambience destroyed."));
    }

    // -----------------------------------------
    // MUSIC INSTANCE PARAMETER
    // -----------------------------------------

    if (Pressed(EKeys::Seven)) SetMusicParameter(0.f, 3.f);
    if (Pressed(EKeys::Eight)) SetMusicParameter(1.f, 3.f);
    if (Pressed(EKeys::Nine)) SetMusicParameter(0.f, 0.f);
    if (Pressed(EKeys::Zero)) SetMusicParameter(1.f, 0.f);

    // -----------------------------------------
    // ONE-SHOTS
    // -----------------------------------------

    if (Pressed(EKeys::Q))
    {
        if (IsValid(TestOneShotEvent)) AudioManager->PlayOneShot(TestOneShotEvent);
        PrintTest(IsValid(TestOneShotEvent) ? TEXT("2D one-shot triggered.") : TEXT("FAILED: Missing 2D event."));
    }

    if (Pressed(EKeys::I))
    {
        if (IsValid(Test3DEvent))
            AudioManager->PlayOneShotAtLocation(Test3DEvent, GetActorLocation() + GetActorForwardVector() * 250.f);

        PrintTest(IsValid(Test3DEvent) ? TEXT("3D one-shot at location.") : TEXT("FAILED: Missing 3D event."));
    }

    if (Pressed(EKeys::E))
    {
        if (IsValid(Test3DEvent))
            AudioManager->PlayOneShotAttached(Test3DEvent, GetRootComponent());

        PrintTest(IsValid(Test3DEvent) ? TEXT("3D one-shot attached.") : TEXT("FAILED: Missing 3D event."));
    }

    // -----------------------------------------
    // ATTACHED LOOPING AUDIO
    // -----------------------------------------

    if (Pressed(EKeys::R))
    {
        const bool bSuccess = EnsureLoop(TestAudioComponent);
        PrintTest(bSuccess ? TEXT("Attached loop playing.") : TEXT("FAILED: Attached loop."));
    }

    if (Pressed(EKeys::T))
    {
        DestroyLoop(TestAudioComponent);
        PrintTest(TEXT("Primary attached loop destroyed."));
    }

    // -----------------------------------------
    // INDEPENDENT INSTANCE TWEENING
    // -----------------------------------------

    if (Pressed(EKeys::O))
    {
        const bool bFirst = EnsureLoop(TestAudioComponent);
        const bool bSecond = EnsureLoop(SecondaryAudioComponent);

        if (bFirst && bSecond)
        {
            AudioManager->SetInstanceParameter(TestAudioComponent, LoopParameterName, 0.f);
            AudioManager->SetInstanceParameter(SecondaryAudioComponent, LoopParameterName, 1.f);
        }

        PrintTest(bFirst && bSecond ? TEXT("Two loops created. A=0, B=1.") : TEXT("FAILED: Creating two loops."));
    }

    if (Pressed(EKeys::U))
    {
        if (IsValid(TestAudioComponent) && IsValid(SecondaryAudioComponent))
        {
            const bool bFirst = AudioManager->SetInstanceParameter(TestAudioComponent, LoopParameterName, 1.f, 4.f);
            const bool bSecond = AudioManager->SetInstanceParameter(SecondaryAudioComponent, LoopParameterName, 0.f, 6.f);

            PrintTest(bFirst && bSecond ? TEXT("Independent tweens: A=>1 (4s), B=>0 (6s).") : TEXT("FAILED: Independent tween."));
        }
        else
        {
            PrintTest(TEXT("Create both loops using O first."));
        }
    }

    if (Pressed(EKeys::Y))
    {
        DestroyLoop(SecondaryAudioComponent);
        PrintTest(TEXT("Secondary loop destroyed. Primary tween should continue."));
    }

    if (Pressed(EKeys::F))
    {
        DestroyLoop(TestAudioComponent);
        DestroyLoop(SecondaryAudioComponent);
        PrintTest(TEXT("Both attached loops destroyed."));
    }

    // -----------------------------------------
    // AUTOMATIC OWNER CLEANUP
    // -----------------------------------------

    if (Pressed(EKeys::G))
    {
        PrintTest(TEXT("Destroying Testbed Actor. Owned audio should stop."));
        Destroy();
        return;
    }

    // -----------------------------------------
    // GLOBAL PARAMETERS
    // -----------------------------------------

    if (Pressed(EKeys::H)) SetGlobal(0.f, 0.f);
    if (Pressed(EKeys::J)) SetGlobal(1.f, 0.f);
    if (Pressed(EKeys::K)) SetGlobal(0.f, 3.f);
    if (Pressed(EKeys::L)) SetGlobal(1.f, 3.f);

    // -----------------------------------------
    // PARAMETER AND MUSIC INSPECTION
    // -----------------------------------------

    if (Pressed(EKeys::P))
    {
        PrintTest(FString::Printf(TEXT("Music: Playing=%d, Paused=%d"), AudioManager->IsMusicPlaying(), AudioManager->IsMusicPaused()));

        float Value = 0.f;

        if (AudioManager->GetGlobalParameter(GlobalParameterName, Value))
            PrintTest(FString::Printf(TEXT("Global %s = %.3f"), *GlobalParameterName.ToString(), Value));

        if (IsValid(AudioManager->GetMusicComponent()) && AudioManager->GetInstanceParameter(AudioManager->GetMusicComponent(), MusicParameterName, Value))
            PrintTest(FString::Printf(TEXT("Music %s = %.3f"), *MusicParameterName.ToString(), Value));

        if (IsValid(TestAudioComponent) && AudioManager->GetInstanceParameter(TestAudioComponent, LoopParameterName, Value))
            PrintTest(FString::Printf(TEXT("Loop A %s = %.3f"), *LoopParameterName.ToString(), Value));

        if (IsValid(SecondaryAudioComponent) && AudioManager->GetInstanceParameter(SecondaryAudioComponent, LoopParameterName, Value))
            PrintTest(FString::Printf(TEXT("Loop B %s = %.3f"), *LoopParameterName.ToString(), Value));
    }

    // -----------------------------------------
    // BUS VOLUME TESTS
    // -----------------------------------------

    if (Pressed(EKeys::Z))
        ToggleVolume(TEXT("Master"), AudioManager->GetMasterVolume(), [this](float V) { return AudioManager->SetMasterVolume(V); });

    if (Pressed(EKeys::X))
        ToggleVolume(TEXT("Music"), AudioManager->GetMusicVolume(), [this](float V) { return AudioManager->SetMusicVolume(V); });

    if (Pressed(EKeys::C))
        ToggleVolume(TEXT("SFX"), AudioManager->GetSFXVolume(), [this](float V) { return AudioManager->SetSFXVolume(V); });

    if (Pressed(EKeys::V))
        ToggleVolume(TEXT("Ambience"), AudioManager->GetAmbienceVolume(), [this](float V) { return AudioManager->SetAmbienceVolume(V); });

    if (Pressed(EKeys::B))
        ToggleVolume(TEXT("UI"), AudioManager->GetUIVolume(), [this](float V) { return AudioManager->SetUIVolume(V); });

    if (Pressed(EKeys::N))
        ToggleVolume(TEXT("Dialogue"), AudioManager->GetDialogueVolume(), [this](float V) { return AudioManager->SetDialogueVolume(V); });
}
