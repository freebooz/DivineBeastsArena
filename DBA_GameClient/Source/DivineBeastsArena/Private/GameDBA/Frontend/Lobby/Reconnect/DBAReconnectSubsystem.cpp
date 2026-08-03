// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#include "GameDBA/Frontend/Lobby/Reconnect/DBAReconnectSubsystem.h"
#include "GameDBA/Frontend/Lobby/Reconnect/DBAReconnectSaveGame.h"
#include "GameDBA/Core/DBALogChannels.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendSessionService.h"
#include "GameBackendTypes.h"

#include "Engine/Engine.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogDBAReconnect, Log, All);

UDBAReconnectSaveGame::UDBAReconnectSaveGame()
{
	SessionId = FString();
	ReconnectToken = FString();
	ReconnectTokenExpiresAt = FString();
}

UDBAReconnectSubsystem::UDBAReconnectSubsystem()
{
}

void UDBAReconnectSubsystem::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	LoadSavedReconnectToken();

	if (HasCachedReconnectToken())
	{
		// 启动时若已有持久化令牌，进入 TokenCached 状态等待 UI 触发重连
		SetReconnectState(EDBAReconnectState::TokenCached, TEXT("检测到上次会话未结束，可执行断线重连。"));
		UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 启动时检测到持久化重连令牌，会话=%s。"), *CachedSessionId);
	}
	else
	{
		UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 启动时未检测到持久化重连令牌。"));
	}
}

void UDBAReconnectSubsystem::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，派生类无额外清理
}

void UDBAReconnectSubsystem::CacheReconnectToken(const FString& SessionId, const FString& ReconnectToken, const FString& ReconnectTokenExpiresAt)
{
	if (SessionId.IsEmpty() || ReconnectToken.IsEmpty())
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 缓存令牌失败：会话 ID 或重连令牌为空。"));
		return;
	}

	CachedSessionId = SessionId;
	CachedReconnectToken = ReconnectToken;
	CachedReconnectTokenExpiresAt = ReconnectTokenExpiresAt;

	SaveReconnectToken();
	SetReconnectState(EDBAReconnectState::TokenCached, TEXT("已缓存重连令牌，可执行断线重连。"));

	UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 已缓存重连令牌，会话=%s 过期=%s。"), *SessionId, *ReconnectTokenExpiresAt);
}

void UDBAReconnectSubsystem::ClearReconnectToken()
{
	CachedSessionId.Reset();
	CachedReconnectToken.Reset();
	CachedReconnectTokenExpiresAt.Reset();

	DeleteSavedReconnectToken();
	SetReconnectState(EDBAReconnectState::Idle, TEXT("已清除重连令牌。"));

	UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 已清除缓存的重连令牌。"));
}

void UDBAReconnectSubsystem::TryReconnect()
{
	if (ReconnectState == EDBAReconnectState::Reconnecting)
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 已有重连请求进行中，忽略重复调用。"));
		return;
	}

	if (!HasCachedReconnectToken())
	{
		SetReconnectState(EDBAReconnectState::Failed, TEXT("无缓存的重连令牌，无法执行断线重连。"));
		return;
	}

	if (IsCachedTokenExpired())
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 重连令牌已过期，会话=%s。"), *CachedSessionId);
		ClearReconnectToken();
		SetReconnectState(EDBAReconnectState::Failed, TEXT("重连令牌已过期，请重新进入匹配。"));
		return;
	}

	UGameInstance* GameInst = GetGameInstance();
	if (!GameInst)
	{
		SetReconnectState(EDBAReconnectState::Failed, TEXT("GameInstance 不可用。"));
		return;
	}

	UDBA_GameBackendClientSubsystem* Backend = GameInst->GetSubsystem<UDBA_GameBackendClientSubsystem>();
	if (!Backend)
	{
		SetReconnectState(EDBAReconnectState::Failed, TEXT("后端客户端子系统不可用。"));
		return;
	}

	UDBA_GameBackendSessionService* SessionService = Backend->GetSessionService();
	if (!SessionService)
	{
		SetReconnectState(EDBAReconnectState::Failed, TEXT("会话服务不可用。"));
		return;
	}

	SetReconnectState(EDBAReconnectState::Reconnecting, TEXT("正在执行断线重连..."));

	UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 发起重连请求，会话=%s。"), *CachedSessionId);

	// 异步调用 Reconnect，不阻塞 GameThread
	FDBA_GameBackendResponseDelegate ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &UDBAReconnectSubsystem::HandleReconnectResponse);

	SessionService->Reconnect(CachedSessionId, CachedReconnectToken, ResponseDelegate);
}

bool UDBAReconnectSubsystem::HasCachedReconnectToken() const
{
	return !CachedSessionId.IsEmpty() && !CachedReconnectToken.IsEmpty();
}

void UDBAReconnectSubsystem::SetReconnectState(EDBAReconnectState NewState, const FString& Message)
{
	if (ReconnectState == NewState)
	{
		// 状态未变也广播消息，便于 UI 显示最新中文说明
	}
	else
	{
		ReconnectState = NewState;
	}

	UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 状态变更：%d 消息：%s"), static_cast<uint8>(NewState), *Message);

	OnReconnectStateChanged.Broadcast(NewState, Message);
}

void UDBAReconnectSubsystem::LoadSavedReconnectToken()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	if (!Loaded)
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 加载持久化重连数据失败。"));
		return;
	}

	UDBAReconnectSaveGame* SaveGame = Cast<UDBAReconnectSaveGame>(Loaded);
	if (!SaveGame)
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 持久化对象类型不匹配。"));
		return;
	}

	SavedReconnectData = SaveGame;
	CachedSessionId = SaveGame->SessionId;
	CachedReconnectToken = SaveGame->ReconnectToken;
	CachedReconnectTokenExpiresAt = SaveGame->ReconnectTokenExpiresAt;
}

void UDBAReconnectSubsystem::SaveReconnectToken()
{
	if (!SavedReconnectData)
	{
		SavedReconnectData = NewObject<UDBAReconnectSaveGame>(this);
	}

	if (!SavedReconnectData)
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 创建持久化对象失败。"));
		return;
	}

	SavedReconnectData->SessionId = CachedSessionId;
	SavedReconnectData->ReconnectToken = CachedReconnectToken;
	SavedReconnectData->ReconnectTokenExpiresAt = CachedReconnectTokenExpiresAt;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SavedReconnectData, SaveSlotName, 0);
	if (!bSaved)
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 持久化重连令牌到本地失败。"));
	}
}

void UDBAReconnectSubsystem::DeleteSavedReconnectToken()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
	}

	SavedReconnectData = nullptr;
}

void UDBAReconnectSubsystem::HandleReconnectResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (bSuccess)
	{
		UE_LOG(LogDBAReconnect, Log, TEXT("[重连] 重连请求成功，响应数据：%s。"), *DataJson);
		SetReconnectState(EDBAReconnectState::Reconnected, TEXT("重连成功，正在恢复游戏会话。"));

		// 重连成功后清除本地持久化的令牌（服务端已重新签发新令牌，由后续 GetConnection 流程重新缓存）
		DeleteSavedReconnectToken();
	}
	else
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 重连请求失败：%s。"), *ErrorMessage);
		SetReconnectState(EDBAReconnectState::Failed, ErrorMessage.IsEmpty() ? TEXT("重连失败，请重新进入匹配。") : ErrorMessage);
	}
}

bool UDBAReconnectSubsystem::IsCachedTokenExpired() const
{
	if (CachedReconnectTokenExpiresAt.IsEmpty())
	{
		// 没有过期时间信息，保守视为未过期，让服务端做最终判定
		return false;
	}

	FDateTime ExpiresAt;
	if (!ParseIso8601(CachedReconnectTokenExpiresAt, ExpiresAt))
	{
		UE_LOG(LogDBAReconnect, Warning, TEXT("[重连] 解析过期时间失败：%s。"), *CachedReconnectTokenExpiresAt);
		return false;
	}

	return FDateTime::UtcNow() >= ExpiresAt;
}

bool UDBAReconnectSubsystem::ParseIso8601(const FString& IsoString, FDateTime& OutDateTime)
{
	// 支持常见 ISO 8601 格式：2026-07-06T13:28:35.2679880Z
	return FDateTime::ParseIso8601(*IsoString, OutDateTime);
}
