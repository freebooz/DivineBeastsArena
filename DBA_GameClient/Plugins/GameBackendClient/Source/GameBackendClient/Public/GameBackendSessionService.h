// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendSessionService.generated.h"

class UDBA_GameBackendClientSubsystem;
class FDBA_GameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UDBA_GameBackendSessionService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	void GetSession(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	void GetConnection(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback);

	/** 为当前已认证玩家和已选择角色申请共享新手村会话。 */
	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	void AllocateVillage(const FString& CharacterId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	void RequestReconnectToken(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback);

	/**
	 * 使用重连令牌执行断线重连。
	 * 客户端应先通过 GetConnection 获取并持久化 ReconnectToken，断线后调用本方法重新获取连接信息。
	 * @param SessionId 会话 ID
	 * @param ReconnectToken GetConnection 返回的重连令牌明文
	 * @param Callback 完成回调
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	void Reconnect(const FString& SessionId, const FString& ReconnectToken, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Session")
	static FString BuildTravelUrl(const FString& Ip, int32 Port, const FString& SessionId, const FString& JoinTicket);

	static FString BuildTravelUrl(
		const FString& Ip,
		int32 Port,
		const FString& SessionId,
		const FString& JoinTicket,
		int32 TeamId,
		const FString& Zodiac,
		const FString& PrimaryElement,
		const FString& FiveCamp,
		const FString& FixedSkillGroupId);

	static FString BuildTravelUrl(
		const FString& Ip,
		int32 Port,
		const FString& SessionId,
		const FString& JoinTicket,
		const FString& PlayerId,
		const FString& CharacterId,
		int32 TeamId,
		const FString& Zodiac,
		const FString& PrimaryElement,
		const FString& FiveCamp,
		const FString& FixedSkillGroupId);

	static bool TryBuildTravelUrlFromConnectionData(
		const FString& ConnectionDataJson,
		const FString& OverrideSessionId,
		FString& OutTravelUrl);

	/** 只解析后台连接 DTO，不执行旅行或修改业务状态。 */
	static bool ParseConnectionData(const FString& DataJson, FDBA_GameBackendSessionConnection& OutConnection);

private:
	TWeakObjectPtr<UDBA_GameBackendClientSubsystem> Subsystem;
	FDBA_GameBackendHttpClient* HttpClient = nullptr;
};
