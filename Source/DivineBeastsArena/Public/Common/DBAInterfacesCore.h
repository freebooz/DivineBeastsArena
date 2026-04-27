// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Common/DBAResultTypes.h"
#include "DBAInterfacesCore.generated.h"

// ============================================================
// 可校验对象接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBAValidatableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可校验对象接口
 * 用于 DataTable、DataAsset、配置对象的数据完整性校验
 */
class DIVINEBEASTSARENA_API IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 校验数据完整性
	 * @param OutErrors 输出错误列表
	 * @return 是否校验通过
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Validation")
	bool ValidateData(TArray<FString> &OutErrors) const;
	virtual bool ValidateData_Implementation(TArray<FString> &OutErrors) const { return true; }
};

// ============================================================
// 可版本化对象接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBAVersionableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可版本化对象接口
 * 用于 SaveGame、Profile、配置文件的版本管理和迁移
 */
class DIVINEBEASTSARENA_API IDBAVersionableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取当前版本号
	 * @return 版本号
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	int32 GetVersion() const;
	virtual int32 GetVersion_Implementation() const { return 1; }

	/**
	 * 从旧版本迁移数据
	 * @param OldVersion 旧版本号
	 * @return 是否迁移成功
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	bool MigrateFromVersion(int32 OldVersion);
	virtual bool MigrateFromVersion_Implementation(int32 OldVersion) { return true; }
};

// ============================================================
// 可调试描述对象接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBADebugDescribableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可调试描述对象接口
 * 用于生成调试信息、日志输出、开发者工具显示
 */
class DIVINEBEASTSARENA_API IDBADebugDescribableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取调试描述字符串
	 * @return 调试描述
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Debug")
	FString GetDebugDescription() const;
	virtual FString GetDebugDescription_Implementation() const { return TEXT("No Description"); }
};

// ============================================================
// 可用性查询接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBAAvailabilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可用性查询接口
 * 用于查询系统、功能、服务的可用状态
 */
class DIVINEBEASTSARENA_API IDBAAvailabilityInterface
{
	GENERATED_BODY()

public:
	/**
	 * 查询是否可用
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Availability")
	bool IsAvailable() const;
	virtual bool IsAvailable_Implementation() const { return true; }

	/**
	 * 获取不可用原因
	 * @return 不可用原因描述
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Availability")
	FString GetUnavailableReason() const;
	virtual FString GetUnavailableReason_Implementation() const { return FString(); }
};

// ============================================================
// 基础交互接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBAInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 基础交互接口
 * 用于世界中可交互对象（传送门、NPC、宝箱等）
 */
class DIVINEBEASTSARENA_API IDBAInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否可交互
	 * @param Interactor 交互发起者
	 * @return 是否可交互
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor *Interactor) const;
	virtual bool CanInteract_Implementation(AActor *Interactor) const { return true; }

	/**
	 * 执行交互
	 * @param Interactor 交互发起者
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor *Interactor);
	virtual void Interact_Implementation(AActor *Interactor) {}

	/**
	 * 获取交互提示文本
	 * @return 提示文本
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const { return FText::FromString(TEXT("交互")); }
};

// ============================================================
// 基础团队代理接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBATeamAgentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 基础团队代理接口
 * 用于标识 Actor 所属团队，支持敌我识别、目标选择
 */
class DIVINEBEASTSARENA_API IDBATeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取团队 ID
	 * @return 团队 ID，-1 表示中立
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	int32 GetTeamId() const;
	virtual int32 GetTeamId_Implementation() const { return -1; }

	/**
	 * 设置团队 ID
	 * @param NewTeamId 新团队 ID
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	void SetTeamId(int32 NewTeamId);
	virtual void SetTeamId_Implementation(int32 NewTeamId) {}

	/**
	 * 判断是否为敌对关系
	 * @param Other 目标对象
	 * @return 是否为敌对
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	bool IsHostileTo(const TScriptInterface<IDBATeamAgentInterface> &Other) const;
	virtual bool IsHostileTo_Implementation(const TScriptInterface<IDBATeamAgentInterface> &Other) const { return false; }
};

// ============================================================
// 基础目标代理接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBATargetableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 基础目标代理接口
 * 用于标识 Actor 是否可被选中、锁定、攻击
 */
class DIVINEBEASTSARENA_API IDBATargetableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否可被选中
	 * @return 是否可被选中
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	bool IsTargetable() const;
	virtual bool IsTargetable_Implementation() const { return true; }

	/**
	 * 获取目标优先级（用于自动选择目标）
	 * @return 优先级，数值越大优先级越高
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	float GetTargetPriority() const;
	virtual float GetTargetPriority_Implementation() const { return 1.0f; }

	/**
	 * 获取目标显示名称
	 * @return 显示名称
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	FText GetTargetDisplayName() const;
	virtual FText GetTargetDisplayName_Implementation() const { return FText::FromString(TEXT("目标")); }
};

// ============================================================
// 可遥测对象接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBATelemetryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可遥测对象接口
 * 用于标识对象支持可选的外部 Monitoring 遥测上报
 * 遥测功能不影响游戏核心逻辑
 */
class DIVINEBEASTSARENA_API IDBATelemetryInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否启用遥测
	 * @return 是否启用
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Telemetry")
	bool IsTelemetryEnabled() const;
	virtual bool IsTelemetryEnabled_Implementation() const { return false; }

	/**
	 * 收集遥测数据
	 * @return 遥测数据（JSON 格式）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Telemetry")
	FString CollectTelemetryData() const;
	virtual FString CollectTelemetryData_Implementation() const { return TEXT("{}"); }
};

// ============================================================
// 可外部服务降级对象接口
// ============================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UDBAExternalServiceDegradableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可外部服务降级对象接口
 * 用于标识对象支持外部服务（Monitoring / GameOps）降级模式
 * 外部服务不可用时，对象必须能够正常工作
 */
class DIVINEBEASTSARENA_API IDBAExternalServiceDegradableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否处于降级模式
	 * @return 是否降级
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	bool IsDegraded() const;
	virtual bool IsDegraded_Implementation() const { return false; }

	/**
	 * 进入降级模式
	 * @param Reason 降级原因
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void EnterDegradedMode(const FString &Reason);
	virtual void EnterDegradedMode_Implementation(const FString &Reason) {}

	/**
	 * 退出降级模式
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void ExitDegradedMode();
	virtual void ExitDegradedMode_Implementation() {}
};