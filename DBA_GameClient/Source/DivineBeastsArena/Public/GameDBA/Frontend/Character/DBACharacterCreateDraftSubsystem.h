// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "DBACharacterCreateDraftSubsystem.generated.h"

class UDBAAppearanceCatalogDataAsset;
class UDBAZodiacHeroDataAsset;
struct FStreamableHandle;

/** 账号角色创建专用步骤；不得用于赛前 Hero/Element/FiveCamp 选择。 */
UENUM(BlueprintType)
enum class EDBACharacterCreateStep : uint8
{
	ZodiacAppearance UMETA(DisplayName = "生肖与外观"),
	Element UMETA(DisplayName = "元素"),
	FiveCamp UMETA(DisplayName = "五营"),
	ConfirmName UMETA(DisplayName = "确认与名称")
};

/** 未创建角色的本地业务草稿，永远不是服务端角色实体。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterCreateDraft
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBACharacterCreateStep CurrentStep = EDBACharacterCreateStep::ZodiacAppearance;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FDBACharacterAppearance Appearance;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAElement ElementType = EDBAElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAFiveCamp FiveCampType = EDBAFiveCamp::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FString CharacterName;

	/** 仅用于展示的已配置技能构筑标识；服务端仍为最终权威。 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FName FixedSkillBuildRowId = NAME_None;

	/** 仅用于显示的属性/构筑摘要，不参与创建权威校验。 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FText PreviewSummary;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FDBAOnCharacterCreateDraftChanged, const FDBACharacterCreateDraft&);

/**
 * 账号角色创建的唯一草稿入口。
 * 生命周期跨前台 Screen 存在；只保存稳定 ID 和本地输入，不保存 Token、Password 或未创建角色的服务端身份。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBACharacterCreateDraftSubsystem final : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;

	const FDBACharacterCreateDraft& GetDraft() const { return Draft; }
	FDBAOnCharacterCreateDraftChanged& OnDraftChanged() { return DraftChanged; }

	void BeginDraft();
	void ResetDraft();
	bool SetZodiac(EDBAZodiac Zodiac);
	bool SetAppearance(const FDBACharacterAppearance& Appearance);
	/** 设置单个外观槽位的稳定 ID；目录和生肖白名单不通过时不改写草稿。 */
	bool SetAppearanceOption(EDBAAppearanceSlot Slot, FName OptionId);
	/** 将当前生肖的外观恢复到 DataAsset 默认组合，再做一次显式合法化。 */
	bool ResetAppearance(FText& OutReason);
	/** 切换生肖、恢复草稿或目录变更后显式清理不再合法的外观 ID。 */
	bool NormalizeAppearance(FText& OutReason);
	/** AppearancePanel 的唯一候选项来源；未配置/未加载目录时返回空列表。 */
	void GetAvailableAppearanceOptionIds(EDBAAppearanceSlot Slot, TArray<FName>& OutOptionIds) const;
	bool RandomizeAppearance();
	/**
	 * 写入由固定技能组规则查询得到的纯展示摘要。
	 * 该数据不属于客户端权威技能装配，最终构筑仍必须由服务端基于 Zodiac + Element 校验。
	 */
	bool SetGeneratedBuildPreview(FName FixedSkillBuildRowId, const FText& InPreviewSummary);
	bool SetElement(EDBAElement Element);
	bool SetFiveCamp(EDBAFiveCamp FiveCamp);
	bool SetCharacterName(const FString& Name);

	bool CanEnter(EDBACharacterCreateStep Step, FText& OutReason) const;
	bool CanLeave(FText& OutReason) const;
	bool Validate(FText& OutReason) const;
	bool Next(FText& OutReason);
	bool Back(FText& OutReason);
	bool BuildCreateRequest(FDBACharacterCreateRequest& OutRequest, FText& OutReason) const;

	/** 供本地临时恢复使用；调用方决定是否写入 SaveGame，草稿本身不触发持久化。 */
	bool SerializeRecovery(FString& OutJson) const;
	bool RestoreRecovery(const FString& Json, FText& OutReason);

private:
	void BroadcastDraftChanged();
	void RequestAppearanceCatalogAsync();
	void RequestZodiacDefaultsAsync(EDBAZodiac Zodiac, uint32 RequestVersion);
	void ApplyZodiacDefaults(const UDBAZodiacHeroDataAsset& ZodiacData, uint32 RequestVersion);
	bool ValidateAppearance(FText& OutReason) const;
	bool IsAppearanceOptionAllowed(FName OptionId) const;
	bool RandomizeAppearanceSlot(EDBAAppearanceSlot Slot, FDBACharacterAppearance& InOutAppearance) const;

	FDBACharacterCreateDraft Draft;
	TWeakObjectPtr<UDBAAppearanceCatalogDataAsset> AppearanceCatalog;
	TSet<FName> AllowedAppearanceOptionIds;
	TMap<FName, FName> DefaultAppearanceOptionIds;
	uint32 DraftRequestVersion = 0;
	TSharedPtr<FStreamableHandle> AppearanceCatalogLoadHandle;
	FDBAOnCharacterCreateDraftChanged DraftChanged;
};
