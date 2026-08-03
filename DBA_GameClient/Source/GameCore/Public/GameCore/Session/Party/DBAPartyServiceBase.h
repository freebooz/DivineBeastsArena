// Copyright FreeboozStudio. All Rights Reserved.
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
#include "GameCore/Session/Party/DBAPartyTypes.h"
#include "DBAPartyServiceBase.generated.h"

/**
 * Party 服务基类
 *
 * 功能：
 * - 创建 Party
 * - 邀请玩家
 * - 接受/拒绝邀请
 * - 离开 Party
 * - 踢出成员
 * - 提升队长
 * - Party 状态同步
 *
 * 实现策略：
 * - 基类提供接口定义和通用逻辑
 * - 派生类实现具体的 Party 逻辑
 * - OnlinePartyService：在线模式，连接外部 Party 服务（未来实现）
 *
 * 外部服务边界：
 * - 外部 Party 服务不可用时返回明确失败结果，不伪造本地队伍状态
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 *
 * Dedicated Server：
 * - Dedicated Server 不调用 Party 接口
 * - Dedicated Server 只验证客户端提供的 Party 信息
 */
UCLASS()
class GAMECORE_API UDBAPartyServiceBase : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAPartyServiceBase();

	// UDBAGameInstanceSubsystemBase interface
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;
	// End of UDBAGameInstanceSubsystemBase interface

	/**
	 * 创建 Party
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void CreateParty(FDBAOnPartyCreated OnComplete);

	/**
	 * 邀请玩家
	 *
	 * @param AccountId 被邀请玩家账户 ID
	 * @param OnComplete 完成回调
	 */
	virtual void InvitePlayer(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete);

	/**
	 * 接受邀请
	 *
	 * @param Invite 邀请信息
	 * @param OnComplete 完成回调
	 */
	virtual void AcceptInvite(const FDBAPartyInvite& Invite, FDBAOnPartyJoined OnComplete);

	/**
	 * 拒绝邀请
	 *
	 * @param Invite 邀请信息
	 * @param OnComplete 完成回调
	 */
	virtual void DeclineInvite(const FDBAPartyInvite& Invite, FDBAOnPartyOperationComplete OnComplete);

	/**
	 * 离开 Party
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void LeaveParty(FDBAOnPartyLeft OnComplete);

	/**
	 * 踢出成员
	 *
	 * @param AccountId 被踢出成员账户 ID
	 * @param OnComplete 完成回调
	 */
	virtual void KickMember(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete);

	/**
	 * 提升队长
	 *
	 * @param AccountId 新队长账户 ID
	 * @param OnComplete 完成回调
	 */
	virtual void PromoteLeader(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete);

	/**
	 * 获取当前 Party 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Party")
	const FDBAPartyInfo& GetCurrentPartyInfo() const { return CurrentPartyInfo; }

	/**
	 * 检查是否在 Party 中
	 */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool IsInParty() const { return CurrentPartyInfo.IsValid(); }

	/**
	 * 检查是否是队长
	 */
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool IsLeader() const;

	/**
	 * Party 更新委托
	 */
	FDBAOnPartyUpdated OnPartyUpdated;

	/**
	 * Party 邀请接收委托
	 */
	FDBAOnPartyInviteReceived OnPartyInviteReceived;

protected:
	/**
	 * 当前 Party 信息
	 */
	UPROPERTY()
	FDBAPartyInfo CurrentPartyInfo;

	/**
	 * 生成 Party ID
	 */
	FDBAPartyId GeneratePartyId();
};
