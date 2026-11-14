#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BugReporterSubsystem.generated.h"

UENUM(BlueprintType)
enum class EBugReportCategory : uint8
{
   None UMETA(DisplayName="Select Category"), Bug,Feedback,Balance
};

USTRUCT(BlueprintType)
struct FBugReportData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EBugReportCategory Category = EBugReportCategory::None;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    FString SystemSpecs;

    UPROPERTY(BlueprintReadOnly)
    FString ScreenshotPath;
    
    UPROPERTY(BlueprintReadOnly)
    FString StackTrace;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBugReportEvent);

UCLASS(Config=BugReporter)
class PROJECTESTIVAL_API UBugReporterSubsystem
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SendToDiscord(const FBugReportData& Report);
    void SendToEmail(const FBugReportData& Report);
    
    UPROPERTY(BlueprintReadOnly,Category="Bug Reporter")
    bool bIsSubmitting = false;
    
    UPROPERTY(BlueprintAssignable)
    FBugReportEvent OnReportSubmitted;

    UPROPERTY(BlueprintAssignable)
    FBugReportEvent OnReportFailed;

    UFUNCTION(BlueprintCallable, Category="Bug Reporter")
    void CaptureScreenshot();

    UFUNCTION(BlueprintCallable, Category="Bug Reporter")
    void SubmitBugReport(
        EBugReportCategory Category,
        const FString& Description
    );

    UFUNCTION(BlueprintPure, Category="Bug Reporter")
    FString GetSystemSpecs() const;

    UFUNCTION(BlueprintPure, Category="Bug Reporter")
    FString GetStackTrace() const;
    
    UFUNCTION(BlueprintPure, Category="Bug Reporter")
    FString GetScreenshotPath() const;

private:

    UPROPERTY(Config)
    FString DiscordWebhookURL;

    UPROPERTY(Config)
    FString EmailAppPassword;

    FString ScreenshotPath;
    void FinishSubmission(bool bSuccess);
};