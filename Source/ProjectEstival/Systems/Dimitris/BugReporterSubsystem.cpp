
#include "BugReporterSubsystem.h"

#include "HAL/FileManager.h"
#include "HAL/PlatformStackWalk.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformMemory.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "curl/curl.h"
#include "UnrealClient.h"
#include "Async/Async.h"
#include <string>
#include "Containers/StringConv.h"

void UBugReporterSubsystem::CaptureScreenshot()
{
    const FString FileName = FString::Printf(TEXT("Bug_%s_%s.png"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")),*FGuid::NewGuid().ToString(EGuidFormats::Digits));
    ScreenshotPath = FPaths::Combine(FPaths::ScreenShotDir(),FileName);

    FScreenshotRequest::RequestScreenshot(ScreenshotPath,false,false);
    UE_LOG(LogTemp,Log,TEXT("Screenshot requested: %s"),*ScreenshotPath);
}

FString UBugReporterSubsystem::GetSystemSpecs() const
{
    const FPlatformMemoryStats Memory = FPlatformMemory::GetStats();
    const uint64 RAM = FPlatformMemory::GetConstants().TotalPhysical;
    const FString GPU = FPlatformMisc::GetPrimaryGPUBrand();

    return FString::Printf(TEXT("OS: %s\nCPU: %s\nGPU/RHI: %s\nRAM: %.2f GB\nAvailable RAM: %.2f GB"),
        *FPlatformMisc::GetOSVersion(),*FPlatformMisc::GetCPUBrand(),*GPU,
        static_cast<double>(RAM)/(1024.0*1024.0*1024.0),
        static_cast<double>(Memory.AvailablePhysical)/(1024.0*1024.0*1024.0));
}

FString UBugReporterSubsystem::GetScreenshotPath() const
{
    return ScreenshotPath;
}

void UBugReporterSubsystem::SubmitBugReport(EBugReportCategory Category,const FString& Description)
{
    if(Description.TrimStartAndEnd().IsEmpty())
    {
        UE_LOG(LogTemp,Warning,TEXT("Bug report description is empty"));
        OnReportFailed.Broadcast();
        return;
    }

    FBugReportData Report;
    Report.Category = Category;
    Report.Description = Description;
    Report.SystemSpecs = GetSystemSpecs();
    Report.ScreenshotPath = ScreenshotPath;
    Report.StackTrace = GetStackTrace();

    if(!IFileManager::Get().FileExists(*Report.ScreenshotPath))
    {
        UE_LOG(LogTemp,Warning,TEXT("Screenshot file not found: %s"),*Report.ScreenshotPath);
        OnReportFailed.Broadcast();
        return;
    }

    const UEnum* CategoryEnum = StaticEnum<EBugReportCategory>();
    const FString CategoryName = CategoryEnum->GetNameStringByValue(static_cast<int64>(Category));

    UE_LOG(LogTemp,Warning,TEXT("===== BUG REPORT ====="));
    UE_LOG(LogTemp,Warning,TEXT("Category: %s"),*CategoryName);
    UE_LOG(LogTemp,Warning,TEXT("Description: %s"),*Description);
    UE_LOG(LogTemp,Warning,TEXT("Specs:\n%s"),*Report.SystemSpecs);
    UE_LOG(LogTemp,Warning,TEXT("Stack Trace:\n%s"),*Report.StackTrace);
    UE_LOG(LogTemp,Warning,TEXT("Screenshot: %s"),*ScreenshotPath);

    SendToDiscord(Report);
    SendToEmail(Report);
}

FString UBugReporterSubsystem::GetStackTrace() const
{
    ANSICHAR StackTrace[65536];
    StackTrace[0] = '\0';

    FPlatformStackWalk::StackWalkAndDump(StackTrace,UE_ARRAY_COUNT(StackTrace),0);
    return ANSI_TO_TCHAR(StackTrace);
}

void UBugReporterSubsystem::SendToDiscord(const FBugReportData& Report)
{
    // Read Discord webhook
    FString WebhookURL;
    const FString ConfigPath = FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("BugReporter.ini"));

    GConfig->LoadFile(ConfigPath);
    UE_LOG(LogTemp,Warning,TEXT("Config Path: %s"),*ConfigPath);
    UE_LOG(LogTemp,Warning,TEXT("File Exists: %s"),IFileManager::Get().FileExists(*ConfigPath) ? TEXT("YES") : TEXT("NO"));

    GConfig->GetString(TEXT("BugReporter"),TEXT("DiscordWebhookURL"),WebhookURL,ConfigPath);

    if(WebhookURL.IsEmpty())
    {
        UE_LOG(LogTemp,Error,TEXT("Discord webhook URL not configured"));
        OnReportFailed.Broadcast();
        return;
    }

    TSharedRef<IHttpRequest,ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(WebhookURL);
    Request->SetVerb(TEXT("POST"));

    const UEnum* CategoryEnum = StaticEnum<EBugReportCategory>();
    const FString CategoryName = CategoryEnum->GetNameStringByValue(static_cast<int64>(Report.Category));

    const FString Message = FString::Printf(
        TEXT("**Bug Report**\n"
             "**Category:** %s\n"
             "**Description:** %s\n\n"
             "**System Specs:**\n%s\n\n"
             "Screenshot and Stack Trace attached below."),
        *CategoryName,*Report.Description,*Report.SystemSpecs);

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("content"),Message.Left(1900));
    JsonObject->SetStringField(TEXT("thread_name"),FString::Printf(TEXT("Bug Report - %s"),*CategoryName));

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(),Writer);

    TArray<uint8> ImageData;
    if(!FFileHelper::LoadFileToArray(ImageData,*Report.ScreenshotPath))
    {
        UE_LOG(LogTemp,Error,TEXT("Failed to load screenshot"));
        OnReportFailed.Broadcast();
        return;
    }

    // Build multipart body
    const FString Boundary = TEXT("EstivalBugReportBoundary");
    const FString LineBreak = TEXT("\r\n");
    TArray<uint8> Body;

    auto AppendString = [&Body](const FString& Text)
    {
        FTCHARToUTF8 Converter(*Text);
        Body.Append(reinterpret_cast<const uint8*>(Converter.Get()),Converter.Length());
    };

    AppendString(TEXT("--") + Boundary + LineBreak +
        TEXT("Content-Disposition: form-data; name=\"payload_json\"") + LineBreak +
        TEXT("Content-Type: application/json") + LineBreak + LineBreak +
        JsonString + LineBreak);

    AppendString(TEXT("--") + Boundary + LineBreak +
        TEXT("Content-Disposition: form-data; name=\"files[0]\"; filename=\"BugScreenshot.png\"") + LineBreak +
        TEXT("Content-Type: image/png") + LineBreak + LineBreak);

    Body.Append(ImageData);
    AppendString(LineBreak);

    AppendString(TEXT("--") + Boundary + LineBreak +
        TEXT("Content-Disposition: form-data; name=\"files[1]\"; filename=\"StackTrace.txt\"") + LineBreak +
        TEXT("Content-Type: text/plain; charset=utf-8") + LineBreak + LineBreak);

    FTCHARToUTF8 StackTraceUtf8(*Report.StackTrace);
    Body.Append(reinterpret_cast<const uint8*>(StackTraceUtf8.Get()),StackTraceUtf8.Length());

    AppendString(LineBreak + TEXT("--") + Boundary + TEXT("--") + LineBreak);

    Request->SetHeader(TEXT("Content-Type"),TEXT("multipart/form-data; boundary=") + Boundary);
    Request->SetContent(Body);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr Req,FHttpResponsePtr Response,bool bSuccess)
        {
            if(bSuccess && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
            {
                UE_LOG(LogTemp,Warning,TEXT("Discord report sent!"));
                OnReportSubmitted.Broadcast();
            }
            else
            {
                const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : -1;
                const FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("No HTTP response");

                UE_LOG(LogTemp,Error,TEXT("Discord report failed! HTTP Code: %d | Response: %s"),StatusCode,*ErrorMessage);
                OnReportFailed.Broadcast();
            }
        });

    Request->ProcessRequest();
}

void UBugReporterSubsystem::SendToEmail(const FBugReportData& Report)
{
    const FString Email = TEXT("saeathenssteambuilds@gmail.com");

    // Read Gmail password
    const FString EmailConfigPath = FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("BugReporter.ini"));
    GConfig->LoadFile(EmailConfigPath);

    FString AppPassword;
    GConfig->GetString(TEXT("Email"),TEXT("AppPassword"),AppPassword,EmailConfigPath);

    if(AppPassword.IsEmpty())
    {
        UE_LOG(LogTemp,Warning,TEXT("SMTP password not configured. Email skipped."));
        return;
    }

    const UEnum* CategoryEnum = StaticEnum<EBugReportCategory>();
    const FString CategoryName = CategoryEnum->GetNameStringByValue(static_cast<int64>(Report.Category));

    const FString EmailBody = FString::Printf(TEXT("Bug Report - Project Estival\n\n")
        TEXT("Category: %s\n\n")
        TEXT("Description:\n%s\n\n")
        TEXT("System Specs:\n%s\n\n")
        TEXT("Stack trace and screenshot are attached.\n"),
        *CategoryName,*Report.Description,*Report.SystemSpecs);

    const FString ScreenshotFile = Report.ScreenshotPath;
    const FString StackTraceText = Report.StackTrace;

    // Send email async
    Async(EAsyncExecution::ThreadPool,
        [Email,AppPassword,CategoryName,EmailBody,ScreenshotFile,StackTraceText]()
        {
            CURL* Curl = curl_easy_init();

            if(!Curl)
            {
                UE_LOG(LogTemp,Error,TEXT("SMTP curl initialization failed."));
                return;
            }

            curl_slist* Recipients = nullptr;
            curl_slist* Headers = nullptr;
            curl_mime* Mime = curl_mime_init(Curl);

            const std::string EmailUtf8 = TCHAR_TO_UTF8(*Email);
            const std::string PasswordUtf8 = TCHAR_TO_UTF8(*AppPassword);
            const std::string ScreenshotUtf8 = TCHAR_TO_UTF8(*ScreenshotFile);
            const std::string BodyUtf8 = TCHAR_TO_UTF8(*EmailBody);
            const std::string StackUtf8 = TCHAR_TO_UTF8(*StackTraceText);

            const FString Subject = TEXT("Bug Report - ") + CategoryName;
            const std::string SubjectUtf8 = TCHAR_TO_UTF8(*Subject);

            const std::string FromHeader = "From: " + EmailUtf8;
            const std::string ToHeader = "To: " + EmailUtf8;
            const std::string SubjectHeader = "Subject: " + SubjectUtf8;

            Headers = curl_slist_append(Headers,FromHeader.c_str());
            Headers = curl_slist_append(Headers,ToHeader.c_str());
            Headers = curl_slist_append(Headers,SubjectHeader.c_str());

            Recipients = curl_slist_append(Recipients,EmailUtf8.c_str());

            // Email body and attachments
            curl_mimepart* TextPart = curl_mime_addpart(Mime);
            curl_mime_data(TextPart,BodyUtf8.c_str(),CURL_ZERO_TERMINATED);
            curl_mime_type(TextPart,"text/plain; charset=utf-8");

            curl_mimepart* ScreenshotPart = curl_mime_addpart(Mime);
            curl_mime_filedata(ScreenshotPart,ScreenshotUtf8.c_str());
            curl_mime_filename(ScreenshotPart,"BugScreenshot.png");
            curl_mime_type(ScreenshotPart,"image/png");
            curl_mime_encoder(ScreenshotPart,"base64");

            curl_mimepart* StackPart = curl_mime_addpart(Mime);
            curl_mime_data(StackPart,StackUtf8.c_str(),CURL_ZERO_TERMINATED);
            curl_mime_filename(StackPart,"StackTrace.txt");
            curl_mime_type(StackPart,"text/plain; charset=utf-8");

            // Gmail SMTP settings
            curl_easy_setopt(Curl,CURLOPT_URL,"smtp://smtp.gmail.com:587");
            curl_easy_setopt(Curl,CURLOPT_USERNAME,EmailUtf8.c_str());
            curl_easy_setopt(Curl,CURLOPT_PASSWORD,PasswordUtf8.c_str());

            curl_easy_setopt(Curl,CURLOPT_USE_SSL,CURLUSESSL_ALL);
            curl_easy_setopt(Curl,CURLOPT_SSL_VERIFYPEER,1L);
            curl_easy_setopt(Curl,CURLOPT_SSL_VERIFYHOST,2L);
            curl_easy_setopt(Curl,CURLOPT_SSL_OPTIONS,CURLSSLOPT_NATIVE_CA);

            curl_easy_setopt(Curl,CURLOPT_MAIL_FROM,EmailUtf8.c_str());
            curl_easy_setopt(Curl,CURLOPT_MAIL_RCPT,Recipients);
            curl_easy_setopt(Curl,CURLOPT_HTTPHEADER,Headers);
            curl_easy_setopt(Curl,CURLOPT_MIMEPOST,Mime);

            curl_easy_setopt(Curl,CURLOPT_CONNECTTIMEOUT,15L);
            curl_easy_setopt(Curl,CURLOPT_TIMEOUT,60L);

            const CURLcode Result = curl_easy_perform(Curl);

            if(Result == CURLE_OK)
            {
                UE_LOG(LogTemp,Warning,TEXT("SMTP email sent successfully!"));
            }
            else
            {
                UE_LOG(LogTemp,Error,TEXT("SMTP email failed: %s"),UTF8_TO_TCHAR(curl_easy_strerror(Result)));
            }

            curl_mime_free(Mime);
            curl_slist_free_all(Recipients);
            curl_slist_free_all(Headers);
            curl_easy_cleanup(Curl);
        });
}
