// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "DBA_GameBackendTelemetryService.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "DBA_GameBackendClientSettings.h"
#include "DBA_GameBackendClientSubsystem.h"
#include "DBA_GameBackendHttpClient.h"
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

void UDBA_GameBackendTelemetryService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;

	const UDBA_GameBackendClientSettings* Settings = GetDefault<UDBA_GameBackendClientSettings>();
	FlushIntervalSeconds = FMath::Max(1.0f, Settings->TelemetryFlushIntervalSeconds);
	MaxQueueSize = FMath::Max(10, Settings->TelemetryMaxQueueSize);
	bTelemetryEnabled = Settings->bEnableTelemetry;

	if (bTelemetryEnabled)
	{
		SetupFlushTimer();
	}
}

void UDBA_GameBackendTelemetryService::Shutdown()
{
	StopFlushTimer();
	FlushInternal(true);
}

void UDBA_GameBackendTelemetryService::TrackEvent(const FString& EventName, const TMap<FString, FString>& Properties)
{
	if (!bTelemetryEnabled)
	{
		return;
	}

	FDBA_GameBackendTelemetryEvent Event;
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

void UDBA_GameBackendTelemetryService::TrackBatch(const TArray<FDBA_GameBackendTelemetryEvent>& Events)
{
	if (!bTelemetryEnabled)
	{
		return;
	}

	for (FDBA_GameBackendTelemetryEvent Event : Events)
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

void UDBA_GameBackendTelemetryService::Flush()
{
	if (!bTelemetryEnabled)
	{
		return;
	}

	FlushInternal(true);
}

void UDBA_GameBackendTelemetryService::SetupFlushTimer()
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
		TimerManager.SetTimer(FlushTimerHandle, this, &UDBA_GameBackendTelemetryService::Flush, FlushIntervalSeconds, true);
	}
}

void UDBA_GameBackendTelemetryService::StopFlushTimer()
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

void UDBA_GameBackendTelemetryService::FlushInternal(bool bForce)
{
	if (!bTelemetryEnabled || !HttpClient || bFlushing || EventQueue.IsEmpty())
	{
		return;
	}

	TArray<FDBA_GameBackendTelemetryEvent> UploadEvents = EventQueue;
	const FString Payload = BuildBatchPayload(UploadEvents);
	bFlushing = true;

	HttpClient->Post(TEXT("/api/telemetry/batch"), Payload, [this, UploadEvents, bForce](const FDBA_GameBackendHttpResult& Result)
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
			UE_LOG(LogDBA_GameBackendClient, Warning, TEXT("埋点强制刷新失败，已保留到队列。消息=%s"), *Result.Message);
		}
	});
}

FString UDBA_GameBackendTelemetryService::BuildBatchPayload(const TArray<FDBA_GameBackendTelemetryEvent>& Events) const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> EventValues;
	EventValues.Reserve(Events.Num());

	for (const FDBA_GameBackendTelemetryEvent& Event : Events)
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
