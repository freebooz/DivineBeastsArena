// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Common/Party/DBAPartyTypes.h"
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
 * - MockPartyService：本地模拟，用于开发和离线测试
 * - OnlinePartyService：在线模式，连接外部 Party 服务（未来实现）
 *
 * 外部服务边界：
 * - 外部 Party 服务不可用时，自动降级到 MockPartyService
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 *
 * Dedicated Server：
 * - Dedicated Server 不调用 Party 接口
 * - Dedicated Server 只验证客户端提供的 Party 信息
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAPartyServiceBase : public UDBAGameInstanceSubsystemBase
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
