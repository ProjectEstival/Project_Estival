
#include "EstivalLogger.h"

#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogEstival);

void EstivalLogger::PrintInternal(
    EType Type,
    ECat Category,
    const float Duration,
    const FString& Message,
    const FColor& Color,
    const TCHAR* File,
    const int32 Line)
{
    const FString FileName = FPaths::GetCleanFilename(File);

    const FString FinalMessage = FString::Printf(
        TEXT("%s [%s:%d] %s"),
        *CategoryToString(Category),
        *FileName,
        Line,
        *Message
    );

    switch (Type)
    {
    case EType::Log:
        UE_LOG(LogEstival, Display, TEXT("%s"), *FinalMessage);
        break;

    case EType::Warning:
        UE_LOG(LogEstival, Warning, TEXT("%s"), *FinalMessage);
        break;

    case EType::Error:
        UE_LOG(LogEstival, Error, TEXT("%s"), *FinalMessage);
        break;

    default:
        break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            Duration,
            Color,
            FinalMessage
        );
    }
}

FString EstivalLogger::CategoryToString(ECat Category)
{
    switch (Category)
    {
    case ECat::AI:
        return TEXT("AI TEAM");

    case ECat::Gameplay:
        return TEXT("GAMEPLAY TEAM");

    case ECat::PCG:
        return TEXT("PCG TEAM");

    case ECat::Audio:
        return TEXT("AUDIO TEAM");

    default:
        return TEXT("UNKNOWN");
    }
}

FColor EstivalLogger::TypeToColor(EType Type)
{
    switch (Type)
    {
    case EType::Log:
        return FColor::White;

    case EType::Warning:
        return FColor::Yellow;

    case EType::Error:
        return FColor::Red;

    default:
        return FColor::White;
    }
}

FColor EstivalLogger::CategoryToColor(ECat Category)
{
    switch (Category)
    {
    case ECat::AI:
        return FColor::White;

    case ECat::Gameplay:
        return FColor::Green;

    case ECat::PCG:
        return FColor::Cyan;

    case ECat::Audio:
        return FColor::Blue;

    default:
        return FColor::White;
    }
}
