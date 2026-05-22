// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化生肖动画配置 - 使用ZodiacType代替多个独立类

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAZodiacAnimConfig_Generic.generated.h"

class UAnimMontage;

/**
 * UDBAZodiacAnimConfig_Generic
 * 泛化生肖动画配置
 * 通过 ZodiacType 从 DataTable 读取配置
 * 替代原有的 UDBAZodiacAnimConfig_<Zodiac> 12个DataAsset类
 *
 * 使用方式:
 * 1. 创建 DataAsset 并设置 ZodiacType
 * 2. 在蓝图中配置对应生肖的动画资源
 * 3. 或使用 DataTable 通过 ZodiacType + AnimationType 查找动画
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimConfig_Generic : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimConfig_Generic();

public:
	/** 生肖类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Idle_Montage;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Walk_Montage;

	/** 奔跑动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Run_Montage;

	/** 普通攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Attack_Montage;

	/** 被动技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Passive_Montage;

	/** Q技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Q_Montage;

	/** W技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> W_Montage;

	/** E技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> E_Montage;

	/** R终极技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> R_Montage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Hit_Montage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Death_Montage;

public:
	/** 根据动画类型获取动画Montage */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	UAnimMontage* GetAnimationByType(FName AnimationType) const;

	/** 获取所有可用动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> GetAllAnimations() const;
};
