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
#include "GameCore/Data/DBADataAssetBase.h"
#include "Engine/DataTable.h"
#include "GameCore/Types/DBACommonTypes.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
#include "GameDBA/Data/Tables/DBAZodiacHeroData.h"
#include "GameDBA/Data/Tables/DBAFixedSkillGroupData.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceRow.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "DBAZodiacHeroDataAsset.generated.h"

class AActor;
class APawn;
class UAnimationAsset;
class UAnimInstance;
class UNiagaraSystem;
class USkeletalMesh;
class USoundBase;
class UTexture2D;

/**
 * 角色预览镜头的静态配置。数值由每个生肖数据资产配置，预览系统只读取而不推导。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacPreviewCameraPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	FRotator CameraRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	float FieldOfView = 0.0f;

	/** 到角色焦点的默认距离；零表示沿用 Preview Camera Rig 的场景配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview", meta = (ClampMin = "0.0"))
	float Distance = 0.0f;

	/** 不同体型的镜头焦点高度偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	float Height = 0.0f;

	/** 缩放下限；零表示沿用 Preview Camera Rig 的场景配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview", meta = (ClampMin = "0.0"))
	float MinDistance = 0.0f;

	/** 缩放上限；零表示沿用 Preview Camera Rig 的场景配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview", meta = (ClampMin = "0.0"))
	float MaxDistance = 0.0f;
};

/**
 * 生肖英雄数据资产
 *
 * 用途：
 * - 集中管理所有生肖英雄数据表
 * - 提供统一的数据查询接口
 * - 支持运行时数据加载和缓存
 *
 * 数据表：
 * - ZodiacHeroDisplayTable：生肖英雄显示数据表
 * - ZodiacHeroConfigTable：生肖英雄配置数据表
 * - FixedSkillGroupTable：固定技能组数据表
 * - AbilitySetSummaryTable：技能组汇总数据表
 *
 * 使用方式：
 * 1. 创建 DataAsset 实例
 * 2. 关联 DataTable 资产
 * 3. 在 GameInstance / Subsystem 中加载
 * 4. 通过接口查询数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacHeroDataAsset : public UDBADataAssetBase, public IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/** DataTable 加载完成事件。参数为加载完成的 DataTable 指针与软引用路径。 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataTableLoaded, UDataTable*, const FSoftObjectPath&);

	UDBAZodiacHeroDataAsset();

	/** 单生肖主资产类型；旧表聚合资产不会注册为该类型。 */
	static const FPrimaryAssetType& GetZodiacHeroPrimaryAssetType();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** 是否为兼容旧选角表的聚合资产。新生肖资产必须为 false。 */
	bool IsLegacyTableCatalog() const { return bLegacyTableCatalog; }

	//~ IDBAValidatableInterface 实现
	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const override;
	//~ IDBAValidatableInterface 实现 结束

	/** DataTable 加载完成事件广播。UI 可订阅此事件驱动更新，避免轮询。 */
	FOnDataTableLoaded OnDataTableLoaded;

	// ==================== 单生肖静态配置（步骤 14 主权威） ====================

	/** 新资产必须关闭该兼容开关，才会作为 ZodiacHero Primary Asset 被 Registry 扫描。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Migration", meta = (DisplayName = "旧表聚合兼容模式"))
	bool bLegacyTableCatalog = true;

	/** 生肖身份；AssetRegistrySearchable 供 Registry 在不加载重资源的前提下建立索引。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "DBA|Zodiac|Identity")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 角色肖像；图标继承自 UDBADataAssetBase::Icon。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** 创建角色第一步展示的定位文案；由生肖 DataAsset 配置，禁止 Widget 硬编码。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	FText CharacterCreateRoleSummary;

	/** 创建角色第一步展示的难度文案；为空时 UI 不显示占位难度。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	FText CharacterCreateDifficulty;

	/** 仅用于前台预览的 Actor 类；Dedicated Server 不得创建该类。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	TSoftClassPtr<AActor> PreviewActorClass;

	/** 对局角色类；TeamId 不从本资产的生肖、元素或五大阵营字段推导。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Gameplay")
	TSoftClassPtr<APawn> GameplayCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Appearance")
	TSoftObjectPtr<USkeletalMesh> BodyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Appearance")
	TSoftObjectPtr<USkeletalMesh> HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Appearance")
	TArray<TSoftObjectPtr<UObject>> DefaultEquipmentAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Animation")
	TSoftClassPtr<UAnimInstance> AnimationBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Animation")
	TSoftObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Animation")
	TSoftObjectPtr<UAnimationAsset> SelectAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	TSoftObjectPtr<UNiagaraSystem> PreviewVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	TSoftObjectPtr<USoundBase> PreviewSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Preview")
	FDBAZodiacPreviewCameraPreset CameraPreset;

	/** 外观槽位 -> 默认选项 ID；只保存 ID，不直接同步加载外观资源。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Appearance")
	TMap<FName, FName> DefaultAppearanceOptionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Appearance")
	TArray<FName> AllowedAppearanceOptionIds;

	/**
	 * 仅为历史资产迁移时的阻断哨兵。该字段不得用于业务；非空即表示仍有旧分类引用，校验会失败。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "DBA|Zodiac|Migration", meta = (DeprecatedProperty, DeprecationMessage = "生肖资产不得携带旧 Faction 或其他旧分类引用。"))
	FName DeprecatedLegacyClassificationId;

	/**
	 * 生肖英雄显示数据表
	 * 包含 12 个生肖英雄的显示信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ZodiacHeroDisplayTable;

	/**
	 * 生肖英雄配置数据表
	 * 包含 12 个生肖英雄的配置信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ZodiacHeroConfigTable;

	/**
	 * 固定技能组数据表
	 * 包含 60 条固定技能组（12 生肖 × 5 元素）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> FixedSkillGroupTable;

	/**
	 * 技能组汇总数据表
	 * 包含 12 个生肖英雄的技能组汇总
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> AbilitySetSummaryTable;

	/** 选角与创建角色界面使用的生肖定位与属性说明表。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables|CharacterSelection")
	TSoftObjectPtr<UDataTable> HeroBalanceTable;

	/** 选角与创建角色界面使用的生肖技能名称表。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables|CharacterSelection")
	TSoftObjectPtr<UDataTable> SkillNameTable;

	/**
	 * 获取生肖英雄显示数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetZodiacHeroDisplayData(EDBAZodiac Zodiac, FDBAZodiacHeroDisplayRow& OutRow) const;

	/**
	 * 获取生肖英雄配置数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetZodiacHeroConfigData(EDBAZodiac Zodiac, FDBAZodiacHeroConfigRow& OutRow) const;

	/**
	 * 获取固定技能组数据
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 自然元素之力类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetFixedSkillGroupData(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutRow) const;

	/**
	 * 获取技能组汇总数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetAbilitySetSummaryData(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutRow) const;

	/**
	 * 异步表已就绪时组装选角展示文本。文本包含角色定位、能力倾向、技能名称和元素技能组说明。
	 * 若数据尚未加载，函数会发起异步请求并返回 false；调用方应订阅 OnDataTableLoaded 刷新界面。
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData|CharacterSelection")
	bool GetCharacterSelectionSummaryText(EDBAZodiac Zodiac, EDBAElement Element, FText& OutText) const;

	/**
	 * 获取所有可用的生肖英雄
	 *
	 * @param OutZodiacs 输出生肖列表
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	void GetAllAvailableZodiacs(TArray<EDBAZodiac>& OutZodiacs) const;

	/**
	 * 检查生肖英雄是否可用
	 *
	 * @param Zodiac 生肖类型
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool IsZodiacAvailable(EDBAZodiac Zodiac) const;

	/**
	 * 检查技能组是否可用
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 自然元素之力类型
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool IsSkillGroupAvailable(EDBAZodiac Zodiac, EDBAElement Element) const;

	/**
	 * 验证数据完整性
	 * 检查所有数据表是否正确配置
	 *
	 * @param OutErrors 输出错误信息列表
	 * @return 是否验证通过
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool ValidateDataIntegrity(TArray<FString>& OutErrors) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	/**
	 * 异步预加载全部生肖英雄数据表。
	 * 查询接口只读取已经加载完成的数据表；未加载时会发起异步请求并返回空结果。
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	void PreloadAllDataTablesAsync() const;

protected:
	/**
	 * 读取已经加载完成的数据表；未加载时只发起异步加载请求，不阻塞线程。
	 *
	 * @param DataTablePtr 数据表软引用
	 * @return 已加载的数据表，未加载或失败返回 nullptr
	 */
	UDataTable* LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	void RequestDataTableAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	bool ValidateLoadedTableRowCount(
		const TSoftObjectPtr<UDataTable>& DataTablePtr,
		const TCHAR* TableLabel,
		int32 ExpectedRowCount,
		TArray<FString>& OutErrors) const;

	/**
	 * 查找数据行
	 *
	 * @param DataTable 数据表
	 * @param RowName 行名称
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	template<typename T>
	bool FindDataRow(UDataTable* DataTable, FName RowName, T& OutRow) const
	{
		if (!DataTable)
		{
			return false;
		}

		T* Row = DataTable->FindRow<T>(RowName, TEXT(""));
		if (!Row)
		{
			return false;
		}

		OutRow = *Row;
		return true;
	}

	/**
	 * 构建生肖行名称
	 * 格式：Zodiac_[ZodiacName]
	 */
	FName BuildZodiacRowName(EDBAZodiac Zodiac) const;

	mutable TSet<FSoftObjectPath> RequestedDataTableLoads;
};
