
#pragma once

#include "CoreMinimal.h"
#include "EstivalLogger.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEstival, Display, All);

UENUM(BlueprintType)
enum class ECat : uint8
{
	AI       UMETA(DisplayName = "AI"),
	Gameplay UMETA(DisplayName = "Gameplay"),
	PCG      UMETA(DisplayName = "PCG"),
	Audio    UMETA(DisplayName = "Audio")
};

UENUM(BlueprintType)
enum class EType : uint8
{
	Log,
	Warning,
	Error
};

class EstivalLogger
{
public:

	static void PrintInternal(
		EType Type,
		ECat Category,
		float Duration,
		const FString& Message,
		const FColor& Color,
		const TCHAR* File,
		int32 Line
	);

	static FString CategoryToString(ECat Category);

	static FColor TypeToColor(EType Type);

	static FColor CategoryToColor(ECat Category);
};

#if !UE_BUILD_SHIPPING

#define PRINTNORMAL(Type, Category, Duration, Format, ...) \
    EstivalLogger::PrintInternal( \
        Type, \
        Category, \
        Duration, \
        FString::Printf(TEXT(Format), ##__VA_ARGS__), \
        EstivalLogger::TypeToColor(Type), \
        TEXT(__FILE__), \
        __LINE__ \
    )

#define ELOG(Category, Duration, Format, ...) \
    PRINTNORMAL(EType::Log, ECat::Category, Duration, Format, ##__VA_ARGS__)

#define EWARN(Category, Duration, Format, ...) \
    PRINTNORMAL(EType::Warning, ECat::Category, Duration, Format, ##__VA_ARGS__)

#define EERROR(Category, Duration, Format, ...) \
    PRINTNORMAL(EType::Error, ECat::Category, Duration, Format, ##__VA_ARGS__)

#else

#define ELOG(Category, Duration, Format, ...) ((void)0)
#define EWARN(Category, Duration, Format, ...) ((void)0)
#define EERROR(Category, Duration, Format, ...) ((void)0)

#endif

