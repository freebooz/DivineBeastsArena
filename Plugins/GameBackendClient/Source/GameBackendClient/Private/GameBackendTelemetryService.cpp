// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendTelemetryService.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameBackendClientSettings.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"

namespace
{
	FString GetUtcNowIso8601()
	{
		return FDateTime::UtcNow().ToIso8601();
	}

	bool IsSensitiveTelemetryKey(const FString& Key)
	{
		FString Normalized = Key.ToLower();
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));

		return Normalized.Contains(TEXT("accesstoken"))
			|| Normalized.Contains(TEXT("refreshtoken"))
			|| Normalized.Contains(TEXT("playersessiontoken"))
			|| Normalized.Contains(TEXT("authorization"));
	}

	bool LooksLikeSensitiveTelemetryValue(const FString& Value)
	{
		const FString Lower = Value.ToLower();
		return Lower.Contains(TEXT("bearer "))
			|| Lower.Contains(TEXT("accesstoken"))
			|| Lower.Contains(TEXT("refreshtoken"))
			|| Lower.Contains(TEXT("playersessiontoken"));
	}

	TMap<FString, FString> SanitizeTelemetryProperties(const TMap<FString, FString>& InProperties)
	{
		TMap<FString, FString> Sanitized;
		for (const TPair<FString, FString>& Pair : InProperties)
		{
			if (IsSensitiveTelemetryKey(Pair.Key) || LooksLikeSensitiveTelemetryValue(Pair.Value))
			{
				continue;
			}

			Sanitized.Add(Pair.Key, Pair.Value);
		}
		return Sanitized;
	}
}

void UGameBackendTelemetryService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;

	const UGameBackendClientSettings* Settings = GetDefault<UGameBackendClientSettings>();
	FlushIntervalSeconds = FMath::Max(1.0f, Settings->TelemetryFlushIntervalSeconds);
	MaxQueueSize = FMath::Max(10, Settings->TelemetryMaxQueueSize);

	SetupFlushTimer();
}

void UGameBackendTelemetryService::Shutdown()
{
	StopFlushTimer();
	FlushInternal(true);
}

void UGameBackendTelemetryService::TrackEvent(const FString& EventName, const TMap<FString, FString>& Properties)
{
	FGameBackendTelemetryEvent Event;
	Event.EventName = EventName;
	Event.Properties = SanitizeTelemetryProperties(Properties);
	Event.TimestampUtc = GetUtcNowIso8601();
	EventQueue.Add(MoveTemp(Event));

	if (EventQueue.Num() > MaxQueueSize)
	{
		const int32 Overflow = EventQueue.Num() - MaxQueueSize;
		EventQueue.RemoveAt(0, Overflow, EAllowShrinking::No);
	}

	if (EventQueue.Num() >= 50)
	{
		FlushInternal(false);
	}
}

void UGameBackendTelemetryService::TrackBatch(const TArray<FGameBackendTelemetryEvent>& Events)
{
	for (FGameBackendTelemetryEvent Event : Events)
	{
		Event.Properties = SanitizeTelemetryProperties(Event.Properties);
		if (Event.TimestampUtc.IsEmpty())
		{
			Event.TimestampUtc = GetUtcNowIso8601();
		}
		EventQueue.Add(MoveTemp(Event));
	}

	if (EventQueue.Num() > MaxQueueSize)
	{
		const int32 Overflow = EventQueue.Num() - MaxQueueSize;
		EventQueue.RemoveAt(0, Overflow, EAllowShrinking::No);
	}

	if (EventQueue.Num() >= 50)
	{
		FlushInternal(false);
	}
}

void UGameBackendTelemetryService::Flush()
{
	FlushInternal(true);
}

void UGameBackendTelemetryService::SetupFlushTimer()
{
	if (!Subsystem.IsValid() || !Subsystem->GetGameInstance())
	{
		return;
	}

	UWorld* World = Subsystem->GetGameInstance()->GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (!TimerManager.TimerExists(FlushTimerHandle))
	{
		TimerManager.SetTimer(FlushTimerHandle, this, &UGameBackendTelemetryService::Flush, FlushIntervalSeconds, true);
	}
}

void UGameBackendTelemetryService::StopFlushTimer()
{
	if (!Subsystem.IsValid() || !Subsystem->GetGameInstance())
	{
		return;
	}

	UWorld* World = Subsystem->GetGameInstance()->GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(FlushTimerHandle);
}

void UGameBackendTelemetryService::FlushInternal(bool bForce)
{
	if (!HttpClient || bFlushing || EventQueue.IsEmpty())
	{
		return;
	}

	TArray<FGameBackendTelemetryEvent> UploadEvents = EventQueue;
	const FString Payload = BuildBatchPayload(UploadEvents);
	bFlushing = true;

	HttpClient->Post(TEXT("/api/telemetry/batch"), Payload, [this, UploadEvents, bForce](const FGameBackendHttpResult& Result)
	{
		bFlushing = false;
		const bool bSuccess = Result.IsSuccessful();
		if (bSuccess)
		{
			EventQueue.RemoveAt(0, UploadEvents.Num(), EAllowShrinking::No);
			return;
		}

		if (bForce)
		{
			UE_LOG(LogGameBackendClient, Warning, TEXT("埋点强制刷新失败，已保留到队列。message=%s"), *Result.Message);
		}
	});
}

FString UGameBackendTelemetryService::BuildBatchPayload(const TArray<FGameBackendTelemetryEvent>& Events) const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> EventValues;
	EventValues.Reserve(Events.Num());

	for (const FGameBackendTelemetryEvent& Event : Events)
	{
		const TSharedRef<FJsonObject> EventObj = MakeShared<FJsonObject>();
		EventObj->SetStringField(TEXT("eventName"), Event.EventName);
		EventObj->SetStringField(TEXT("timestampUtc"), Event.TimestampUtc);

		const TSharedRef<FJsonObject> PropsObj = MakeShared<FJsonObject>();
		for (const TPair<FString, FString>& Pair : Event.Properties)
		{
			PropsObj->SetStringField(Pair.Key, Pair.Value);
		}
		EventObj->SetObjectField(TEXT("properties"), PropsObj);
		EventValues.Add(MakeShared<FJsonValueObject>(EventObj));
	}

	Root->SetArrayField(TEXT("events"), EventValues);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Root, Writer);
	return Body;
}
