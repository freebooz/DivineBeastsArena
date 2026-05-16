// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendCrashService.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "GameBackendHttpClient.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString GetUploadMarkPath(const FString& FilePath)
	{
		return FilePath + TEXT(".uploaded");
	}
}

void UGameBackendCrashService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendCrashService::ScanCrashFiles()
{
	const FString CrashRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Crashes"));
	if (!FPaths::DirectoryExists(CrashRoot))
	{
		return;
	}

	TArray<FString> CrashDirs;
	IFileManager::Get().FindFilesRecursive(CrashDirs, *CrashRoot, TEXT("*"), false, true, false);
	for (const FString& Dir : CrashDirs)
	{
		UploadCrashDirectory(Dir);
	}

	TryUploadLatestClientLog();
}

void UGameBackendCrashService::UploadCrashReport(const FString& CrashFilePath, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("崩溃服务不可用。"), TEXT("{}"));
		return;
	}

	if (IsAlreadyUploaded(CrashFilePath))
	{
		Callback.ExecuteIfBound(true, FString(), TEXT("{\"alreadyUploaded\":true}"));
		return;
	}

	Async(EAsyncExecution::ThreadPool, [this, CrashFilePath, Callback]()
	{
		FString Error;
		const FString Payload = BuildFileUploadPayload(CrashFilePath, MAX_int64, Error);
		if (Payload.IsEmpty())
		{
			AsyncTask(ENamedThreads::GameThread, [Callback, Error]()
			{
				Callback.ExecuteIfBound(false, Error.IsEmpty() ? TEXT("崩溃文件读取失败。") : Error, TEXT("{}"));
			});
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [this, CrashFilePath, Payload, Callback]()
		{
			HttpClient->Post(TEXT("/api/crashes/upload"), Payload, [this, CrashFilePath, Callback](const FGameBackendHttpResult& Result)
			{
				const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
				if (bSuccess)
				{
					MarkUploaded(CrashFilePath);
				}

				Callback.ExecuteIfBound(bSuccess, bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("崩溃上报失败。") : Result.Message), Result.DataJson);
			});
		});
	});
}

void UGameBackendCrashService::UploadClientLog(const FString& LogFilePath, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("日志上传服务不可用。"), TEXT("{}"));
		return;
	}

	if (IsAlreadyUploaded(LogFilePath))
	{
		Callback.ExecuteIfBound(true, FString(), TEXT("{\"alreadyUploaded\":true}"));
		return;
	}

	Async(EAsyncExecution::ThreadPool, [this, LogFilePath, Callback]()
	{
		FString Error;
		const FString Payload = BuildFileUploadPayload(LogFilePath, MaxLogUploadBytes, Error);
		if (Payload.IsEmpty())
		{
			AsyncTask(ENamedThreads::GameThread, [Callback, Error]()
			{
				Callback.ExecuteIfBound(false, Error.IsEmpty() ? TEXT("日志文件读取失败。") : Error, TEXT("{}"));
			});
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [this, LogFilePath, Payload, Callback]()
		{
			HttpClient->Post(TEXT("/api/client-logs/upload"), Payload, [this, LogFilePath, Callback](const FGameBackendHttpResult& Result)
			{
				const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
				if (bSuccess)
				{
					MarkUploaded(LogFilePath);
				}

				Callback.ExecuteIfBound(bSuccess, bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("日志上传失败。") : Result.Message), Result.DataJson);
			});
		});
	});
}

void UGameBackendCrashService::UploadCrashDirectory(const FString& CrashDir)
{
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *CrashDir, TEXT("*.*"), true, false, false);
	for (const FString& FilePath : Files)
	{
		if (IsAlreadyUploaded(FilePath))
		{
			continue;
		}

		FGameBackendResponseDelegate Noop;
		UploadCrashReport(FilePath, Noop);
	}
}

void UGameBackendCrashService::TryUploadLatestClientLog()
{
	const FString LogsRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Logs"));
	TArray<FString> LogFiles;
	IFileManager::Get().FindFilesRecursive(LogFiles, *LogsRoot, TEXT("*.log"), true, false, false);
	if (LogFiles.IsEmpty())
	{
		return;
	}

	LogFiles.Sort([](const FString& A, const FString& B)
	{
		return IFileManager::Get().GetTimeStamp(*A) > IFileManager::Get().GetTimeStamp(*B);
	});

	for (const FString& LogFile : LogFiles)
	{
		if (IsAlreadyUploaded(LogFile))
		{
			continue;
		}

		FGameBackendResponseDelegate Noop;
		UploadClientLog(LogFile, Noop);
		break;
	}
}

bool UGameBackendCrashService::IsAlreadyUploaded(const FString& FilePath) const
{
	return IFileManager::Get().FileExists(*GetUploadMarkPath(FilePath));
}

void UGameBackendCrashService::MarkUploaded(const FString& FilePath) const
{
	FFileHelper::SaveStringToFile(TEXT("uploaded=true"), *GetUploadMarkPath(FilePath), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

FString UGameBackendCrashService::BuildFileUploadPayload(const FString& FilePath, int64 MaxBytes, FString& OutError) const
{
	if (!FPaths::FileExists(FilePath))
	{
		OutError = TEXT("文件不存在。");
		return FString();
	}

	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
	{
		OutError = TEXT("读取文件失败。");
		return FString();
	}

	if (MaxBytes > 0 && Bytes.Num() > MaxBytes)
	{
		OutError = FString::Printf(TEXT("文件过大，最大支持 %lld 字节。"), MaxBytes);
		return FString();
	}

	const FString Base64 = FBase64::Encode(Bytes);
	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("fileName"), FPaths::GetCleanFilename(FilePath));
	Json->SetStringField(TEXT("filePath"), FilePath);
	Json->SetStringField(TEXT("fileBase64"), Base64);

	FString Out;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Json, Writer);
	return Out;
}

