// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Common/Party/DBAPartyTypes.h"
#include "Common/Queue/DBAQueueTypes.h"
#include "Common/Queue/DBAReadyCheckTypes.h"
#include "Common/Session/DBAMatchSessionTypes.h"
#include "Common/Session/DBATravelTypes.h"
#include "DBAFrontendSessionSubsystem.generated.h"

/**
 * 前台会话状态
 * 管理玩家在前台的整体状态
 */
UENUM(BlueprintType)
enum class EDBAFrontendSessionState : uint8
{
	/** 未初始化 */
	None = 0 UMETA(DisplayName = "未初始化"),

	/** 登录中 */
	LoggingIn UMETA(DisplayName = "登录中"),

	/** 主大厅 */
	MainLobby UMETA(DisplayName = "主大厅"),

	/** 新手村 */
	NewbieVillage UMETA(DisplayName = "新手村"),

	/** 组队中 */
	InParty UMETA(DisplayName = "组队中"),

	/** 匹配中 */
	InQueue UMETA(DisplayName = "匹配中"),

	/** 准备确认中 */
	InReadyCheck UMETA(DisplayName = "准备确认中"),

	/** 加载中 */
	Loading UMETA(DisplayName = "加载中"),

	/** 对局中 */
	InMatch UMETA(DisplayName = "对局中")
};

/**
 * 前台会话子系统
 *
 * 功能：
 * - 管理前台会话状态
 * - 协调 Party / Queue / ReadyCheck / Travel 流程
 * - 提供状态查询接口
 * - 处理状态转换
 *
 * 状态归属矩阵：
 * - Party：MainLobby / NewbieVillage / InParty
 * - Queue：InQueue
 * - ReadyCheck：InReadyCheck
 * - MatchFound：InReadyCheck -> Loading
 * - TravelPreparation：Loading
 * - InMatch：对局中（不在前台管理范围）
 *
 * 与第11部分衔接：
 * - 登录成功后进入 MainLobby
 * - 角色选择完成后可以创建/加入 Party
 * - Party 可以开始 Queue
 *
 * Mock / Local 模式兜底：
 * - 外部服务不可用时使用本地模拟
 * - 本地模拟支持完整的 Party / Queue / ReadyCheck 流程
 * - 本地模拟使用本地 Dedicated Server（如果可用）
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAFrontendSessionSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAFrontendSessionSubsystem();

	// UDBAGameInstanceSubsystemBase interface
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;
	// End of UDBAGameInstanceSubsystemBase interface

	/**
	 * 获取当前会话状态
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	EDBAFrontendSessionState GetCurrentState() const { return CurrentState; }

	/**
	 * 设置会话状态
	 *
	 * @param NewState 新状态
	 */
	void SetState(EDBAFrontendSessionState NewState);

	/**
	 * 获取当前 Party 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	const FDBAPartyInfo& GetCurrentPartyInfo() const { return CurrentPartyInfo; }

	/**
	 * 设置当前 Party 信息
	 */
	void SetCurrentPartyInfo(const FDBAPartyInfo& PartyInfo);

	/**
	 * 清空当前 Party 信息
	 */
	void ClearCurrentPartyInfo();

	/**
	 * 获取当前 Queue 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	const FDBAQueueInfo& GetCurrentQueueInfo() const { return CurrentQueueInfo; }

	/**
	 * 设置当前 Queue 信息
	 */
	void SetCurrentQueueInfo(const FDBAQueueInfo& QueueInfo);

	/**
	 * 清空当前 Queue 信息
	 */
	void ClearCurrentQueueInfo();

	/**
	 * 获取当前 ReadyCheck 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	const FDBAReadyCheckInfo& GetCurrentReadyCheckInfo() const { return CurrentReadyCheckInfo; }

	/**
	 * 设置当前 ReadyCheck 信息
	 */
	void SetCurrentReadyCheckInfo(const FDBAReadyCheckInfo& ReadyCheckInfo);

	/**
	 * 清空当前 ReadyCheck 信息
	 */
	void ClearCurrentReadyCheckInfo();

	/**
	 * 获取当前 MatchSession 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	const FDBAMatchSessionInfo& GetCurrentMatchSessionInfo() const { return CurrentMatchSessionInfo; }

	/**
	 * 设置当前 MatchSession 信息
	 */
	void SetCurrentMatchSessionInfo(const FDBAMatchSessionInfo& SessionInfo);

	/**
	 * 清空当前 MatchSession 信息
	 */
	void ClearCurrentMatchSessionInfo();

	/**
	 * 获取当前 Travel 上下文
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	const FDBATravelContext& GetCurrentTravelContext() const { return CurrentTravelContext; }

	/**
	 * 设置当前 Travel 上下文
	 */
	void SetCurrentTravelContext(const FDBATravelContext& Context);

	/**
	 * 清空当前 Travel 上下文
	 */
	void ClearCurrentTravelContext();

	/**
	 * 检查是否在 Party 中
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	bool IsInParty() const { return CurrentPartyInfo.IsValid(); }

	/**
	 * 检查是否在 Queue 中
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	bool IsInQueue() const { return CurrentQueueInfo.IsValid(); }

	/**
	 * 检查是否在 ReadyCheck 中
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	bool IsInReadyCheck() const { return CurrentReadyCheckInfo.IsValid(); }

	/**
	 * 检查是否在 MatchSession 中
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	bool IsInMatchSession() const { return CurrentMatchSessionInfo.IsValid(); }

	/**
	 * 重置会话
	 * 清空所有状态，返回 MainLobby
	 */
	UFUNCTION(BlueprintCallable, Category = "FrontendSession")
	void ResetSession();

	/**
	 * 会话状态变化委托
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnSessionStateChanged, EDBAFrontendSessionState /* OldState */, EDBAFrontendSessionState /* NewState */);
	FDBAOnSessionStateChanged OnSessionStateChanged;

protected:
	/**
	 * 当前会话状态
	 */
	UPROPERTY()
	EDBAFrontendSessionState CurrentState = EDBAFrontendSessionState::None;

	/**
	 * 当前 Party 信息
	 */
	UPROPERTY()
	FDBAPartyInfo CurrentPartyInfo;

	/**
	 * 当前 Queue 信息
	 */
	UPROPERTY()
	FDBAQueueInfo CurrentQueueInfo;

	/**
	 * 当前 ReadyCheck 信息
	 */
	UPROPERTY()
	FDBAReadyCheckInfo CurrentReadyCheckInfo;

	/**
	 * 当前 MatchSession 信息
	 */
	UPROPERTY()
	FDBAMatchSessionInfo CurrentMatchSessionInfo;

	/**
	 * 当前 Travel 上下文
	 */
	UPROPERTY()
	FDBATravelContext CurrentTravelContext;
};
