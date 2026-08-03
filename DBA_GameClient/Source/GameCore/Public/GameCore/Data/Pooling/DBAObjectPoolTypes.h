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
#include "UObject/Object.h"
#include "DBAObjectPoolTypes.generated.h"

/**
 * 对象池类型
 * 用于区分不同类型的池化对象
 */
UENUM(BlueprintType)
enum class EDBAObjectPoolType : uint8
{
	/** 未定义 */
	None = 0 UMETA(DisplayName = "未定义"),

	/** 抛射物（Projectile） */
	Projectile UMETA(DisplayName = "抛射物"),

	/** 伤害数字（DamageNumber） */
	DamageNumber UMETA(DisplayName = "伤害数字"),

	/** 头顶血条（OverheadBar） */
	OverheadBar UMETA(DisplayName = "头顶血条"),

	/** 通知（Notification） */
	Notification UMETA(DisplayName = "通知"),

	/** 战斗文本（CombatText） */
	CombatText UMETA(DisplayName = "战斗文本"),

	/** 通用 Widget */
	Widget UMETA(DisplayName = "通用Widget"),

	/** 音效（AudioComponent） */
	Audio UMETA(DisplayName = "音效"),

	/** 粒子特效（NiagaraComponent） */
	Niagara UMETA(DisplayName = "粒子特效"),

	/** 自定义类型 */
	Custom UMETA(DisplayName = "自定义类型")
};

/**
 * 池化对象接口
 *
 * 所有需要池化的对象必须实现此接口
 * 提供对象激活、反激活、重置的标准方法
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAPoolableObject : public UInterface
{
	GENERATED_BODY()
};

class GAMECORE_API IDBAPoolableObject
{
	GENERATED_BODY()

public:
	/**
	 * 从对象池中取出时调用
	 * 用于初始化对象状态
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnAcquiredFromPool();

	/**
	 * 归还到对象池时调用
	 * 用于清理对象状态，准备下次使用
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnReturnedToPool();

	/**
	 * 重置对象状态
	 * 用于清理所有运行时数据，恢复到初始状态
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void ResetPoolableObject();

	/**
	 * 检查对象是否可以归还到池中
	 * 某些情况下对象可能处于不可归还状态（例如正在播放动画）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	bool CanReturnToPool() const;
};

/**
 * 对象池配置
 * 用于配置对象池的行为
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAObjectPoolConfig
{
	GENERATED_BODY()

	/** 池类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	EDBAObjectPoolType PoolType = EDBAObjectPoolType::None;

	/** 池化对象类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	TSubclassOf<UObject> ObjectClass;

	/** 初始池大小
	 * 对象池创建时预分配的对象数量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (ClampMin = "0", ClampMax = "1000"))
	int32 InitialPoolSize = 10;

	/** 最大池大小
	 * 对象池最多可以容纳的对象数量
	 * 0 表示无限制
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (ClampMin = "0", ClampMax = "10000"))
	int32 MaxPoolSize = 100;

	/** 是否允许动态扩展
	 * 当池中对象不足时，是否允许创建新对象
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bAllowDynamicExpansion = true;

	/** 是否在归还时重置对象
	 * 如果为 true，归还时会调用 ResetPoolableObject()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bResetOnReturn = true;

	/** 是否在 Dedicated Server 创建池
	 * 某些池（如 UI Widget 池）在 Dedicated Server 不需要创建
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bCreateOnDedicatedServer = true;

	/** Android 特殊配置：最大池大小缩减比例
	 * Android 设备内存有限，可以缩减池大小
	 * 例如：0.5 表示 Android 上池大小为 PC 的 50%
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|Android", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AndroidPoolSizeScale = 0.6f;

	FDBAObjectPoolConfig()
	{
	}
};

/**
 * 对象池统计信息
 * 用于调试和性能监控
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAObjectPoolStats
{
	GENERATED_BODY()

	/** 池类型 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	EDBAObjectPoolType PoolType = EDBAObjectPoolType::None;

	/** 总对象数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 TotalObjects = 0;

	/** 可用对象数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 AvailableObjects = 0;

	/** 使用中对象数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 ActiveObjects = 0;

	/** 累计获取次数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 TotalAcquireCount = 0;

	/** 累计归还次数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 TotalReturnCount = 0;

	/** 累计动态创建次数 */
	UPROPERTY(BlueprintReadOnly, Category = "ObjectPool")
	int32 TotalDynamicCreateCount = 0;

	FDBAObjectPoolStats()
	{
	}
};

/**
 * 单个对象池数据
 */
USTRUCT()
struct GAMECORE_API FObjectPoolData
{
	GENERATED_BODY()

	/** 池配置 */
	UPROPERTY()
	FDBAObjectPoolConfig Config;

	/** 可用对象列表 */
	UPROPERTY()
	TArray<UObject*> AvailableObjects;

	/** 使用中对象列表 */
	UPROPERTY()
	TArray<UObject*> ActiveObjects;

	/** 统计信息 */
	UPROPERTY()
	FDBAObjectPoolStats Stats;
};
