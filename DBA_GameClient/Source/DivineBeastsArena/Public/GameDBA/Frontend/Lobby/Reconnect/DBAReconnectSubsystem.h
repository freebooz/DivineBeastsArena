// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBAReconnectSubsystem.generated.h"

class UDBA_GameBackendSessionService;
class UDBAReconnectSaveGame;

/**
 * 断线重连状态
 */
UENUM(BlueprintType)
enum class EDBAReconnectState : uint8
{
	/** 空闲：没有需要重连的会话 */
	Idle UMETA(DisplayName = "空闲"),
	/** 已持久化重连令牌：可执行重连 */
	TokenCached UMETA(DisplayName = "令牌已缓存"),
	/** 正在执行重连请求 */
	Reconnecting UMETA(DisplayName = "重连中"),
	/** 重连成功 */
	Reconnected UMETA(DisplayName = "重连成功"),
	/** 重连失败（令牌过期 / 会话已结束 / 网络错误） */
	Failed UMETA(DisplayName = "重连失败"),
};

/**
 * 重连状态变更委托
 * @param NewState 新状态
 * @param Message 中文说明
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAReconnectStateChangedDelegate, EDBAReconnectState, NewState, const FString&, Message);

/**
 * UDBAReconnectSubsystem
 *
 * 断线重连子系统
 *
 * 职责：
 * - 持久化玩家当前会话的 ReconnectToken（USaveGame）
 * - 在 GetConnection 成功后缓存 ReconnectToken
 * - 检测断线后自动调用 GameBackendSessionService::Reconnect
 * - 通过事件驱动委托通知 UI 层展示重连进度
 *
 * 设计原则：
 * - 不在 UI 层轮询状态，全部通过 FDBAReconnectStateChangedDelegate 事件驱动
 * - 所有 HTTP 调用通过 GameBackendSessionService 异步执行，不阻塞 GameThread
 * - ReconnectToken 仅在内存与本地 SaveGame 中保存，不上传任何第三方
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAReconnectSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAReconnectSubsystem();

protected:
	// P1-1 改造：重写项目基类生命周期钩子，替代原生 Initialize/Deinitialize
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

	/**
	 * 在 GetConnection 成功后缓存重连令牌。
	 * 同时持久化到本地 SaveGame，保证进程重启后仍可重连。
	 * @param SessionId 会话 ID
	 * @param ReconnectToken GetConnection 返回的重连令牌明文
	 * @param ReconnectTokenExpiresAt 重连令牌过期时间（ISO 8601 字符串）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Reconnect")
	void CacheReconnectToken(const FString& SessionId, const FString& ReconnectToken, const FString& ReconnectTokenExpiresAt);

	/**
	 * 清除缓存的重连令牌（重连成功 / 会话结束 / 玩家主动退出时调用）。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Reconnect")
	void ClearReconnectToken();

	/**
	 * 尝试执行断线重连。
	 * 内部调用 GameBackendSessionService::Reconnect，结果通过 OnReconnectStateChanged 广播。
	 * 若无缓存令牌或令牌已过期，立即广播 Failed 状态。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Reconnect")
	void TryReconnect();

	/**
	 * 获取当前重连状态。
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|Reconnect")
	EDBAReconnectState GetReconnectState() const { return ReconnectState; }

	/**
	 * 是否有可用的缓存重连令牌。
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|Reconnect")
	bool HasCachedReconnectToken() const;

	/**
	 * 获取缓存的会话 ID。
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|Reconnect")
	const FString& GetCachedSessionId() const { return CachedSessionId; }

public:
	/** 重连状态变更事件，UI 层订阅此委托更新表现 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Reconnect")
	FDBAReconnectStateChangedDelegate OnReconnectStateChanged;

private:
	/** 切换状态并广播事件 */
	void SetReconnectState(EDBAReconnectState NewState, const FString& Message);

	/** 加载本地持久化的重连令牌 */
	void LoadSavedReconnectToken();

	/** 保存重连令牌到本地 SaveGame */
	void SaveReconnectToken();

	/** 删除本地 SaveGame */
	void DeleteSavedReconnectToken();

	/** 处理 Reconnect HTTP 响应 */
	void HandleReconnectResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	/** 检查缓存令牌是否过期 */
	bool IsCachedTokenExpired() const;

	/** 解析 ISO 8601 时间字符串为 FDateTime */
	static bool ParseIso8601(const FString& IsoString, FDateTime& OutDateTime);

private:
	/** 当前重连状态 */
	UPROPERTY(Transient)
	EDBAReconnectState ReconnectState = EDBAReconnectState::Idle;

	/** 缓存的会话 ID */
	UPROPERTY(Transient)
	FString CachedSessionId;

	/** 缓存的重连令牌明文（仅内存） */
	UPROPERTY(Transient)
	FString CachedReconnectToken;

	/** 缓存的重连令牌过期时间（ISO 8601） */
	UPROPERTY(Transient)
	FString CachedReconnectTokenExpiresAt;

	/** 持久化的 SaveGame 对象 */
	UPROPERTY(Transient)
	TObjectPtr<UDBAReconnectSaveGame> SavedReconnectData;

	/** SaveGame 槽位名称 */
	static constexpr const TCHAR* SaveSlotName = TEXT("DBAReconnect");
};
