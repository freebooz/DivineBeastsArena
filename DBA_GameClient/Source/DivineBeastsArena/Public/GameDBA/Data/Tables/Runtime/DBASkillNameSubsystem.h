// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明技能名称 Subsystem，异步加载 DataTable 并提供按生肖/槽位查询技能名称的接口。
- 阅读重点：Initialize 异步加载入口、GetSkillName 查询接口、IsDataTableReady 就绪检查。
- 修改提示：新增查询接口时确保未加载时返回默认值并输出中文警告，不阻塞 GameThread。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBASkillNameSubsystem.generated.h"

class UDataTable;
struct FDBASkillNameRow;
struct FStreamableHandle;

/**
 * 技能名称 Subsystem
 *
 * 用途：
 * - 替代 DBAConstants::DBASkillNames 命名空间中的硬编码技能名称常量
 * - 异步加载 DT_SkillNames DataTable，提供运行时查询接口
 * - UI 可通过 GetSkillName 获取本地化技能名称，避免硬编码
 *
 * 使用方式：
 * - GameInstance 启动时自动 Initialize，从 DeveloperSettings 读取 DataTable 软引用
 * - 调用 GetSkillName / GetAllSkillNamesByZodiac 查询
 * - 未加载完成时返回 false 并输出中文警告，不阻塞调用方
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBASkillNameSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

protected:
	// P1-1 改造：重写项目基类生命周期钩子，替代原生 Initialize/Deinitialize
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

	/**
	 * 获取指定生肖和槽位的技能名称
	 *
	 * @param Zodiac 生肖类型
	 * @param SkillSlotIndex 技能槽位索引（0=Passive, 1~4=Skill01~04, 5=Ultimate）
	 * @param OutName 输出技能名称
	 * @return 是否查询成功（DataTable 未就绪或行不存在返回 false）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillName")
	bool GetSkillName(EDBAZodiacType Zodiac, int32 SkillSlotIndex, FText& OutName) const;

	/**
	 * 获取指定生肖的全部 6 个技能名称（按槽位索引 0~5 顺序）
	 *
	 * @param Zodiac 生肖类型
	 * @param OutNames 输出技能名称数组（长度固定为 6）
	 * @return 是否查询成功
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillName")
	bool GetAllSkillNamesByZodiac(EDBAZodiacType Zodiac, TArray<FText>& OutNames) const;

	/** DataTable 是否已加载就绪 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillName")
	bool IsDataTableReady() const;

private:
	/** 从 DeveloperSettings 读取软引用并发起异步加载 */
	void InitializeWithSettings();

	/** DataTable 异步加载完成回调 */
	void HandleDataTableLoaded();

	/** 根据 Zodiac + SlotIndex 查找行 */
	const FDBASkillNameRow* FindRowByZodiac(EDBAZodiacType Zodiac, int32 SkillSlotIndex) const;

	/** 已加载的 DataTable 缓存（弱引用，不阻止 GC） */
	UPROPERTY(Transient)
	TWeakObjectPtr<UDataTable> CachedDataTable;

	/** 异步加载句柄，加载完成或 Subsystem 析构时释放 */
	TSharedPtr<FStreamableHandle> DataTableStreamableHandle;

	/** 是否已输出过未就绪警告（避免日志刷屏） */
	mutable bool bHasLoggedTableNotReady = false;
};
