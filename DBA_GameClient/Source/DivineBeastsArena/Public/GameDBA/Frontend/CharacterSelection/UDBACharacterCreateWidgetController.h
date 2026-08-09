// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBACharacterCreateWidgetController.generated.h"

class UDBAFrontendFlowSubsystem;
class UDBACharacterCreateDraftSubsystem;
class UDBAZodiacRegistrySubsystem;
class UDBACharacterPreviewSubsystem;
class UDBASkillGroupGeneratorSubsystem;
class UDBACharacterCreateZodiacViewModel;
class UDBACharacterCreateElementViewModel;
class UDBAZodiacHeroDataAsset;
struct FDBACharacterCreateDraft;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZodiacStepError, const FText&, ErrorMessage);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetCharacterName(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetZodiac(EDBAZodiac InZodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetElement(EDBAElement InElement);

	/** 元素选择只更新 Draft，固定构筑摘要由规则子系统在 DraftChanged 后生成。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	void SelectElement(EDBAElement InElement);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetFiveCamp(EDBAFiveCamp InFiveCamp);

	/** 绑定生肖第一步需要的 Draft、Registry 与 Preview 事件；反复调用会先安全解绑。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Zodiac")
	void BindZodiacStep();

	/** 绑定第二步元素与固定构筑展示；重复调用会安全解除旧订阅后重建投影。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	void BindElementStep();

	/** 选择生肖只更新 Draft，预览与动态外观面板由 DraftChanged 事件异步刷新。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Zodiac")
	void SelectZodiac(EDBAZodiac InZodiac);

	/** 仅接受 Catalog/Draft 已过滤后的稳定外观 ID。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	bool SelectAppearanceOption(EDBAAppearanceSlot Slot, FName OptionId);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	bool ResetAppearance();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	bool RandomizeAppearance();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	bool Next();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void Back();

	/** Preview 控制只委托给 PreviewSubsystem，Widget 不直接查找或旋转 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void RotatePreview(float DeltaYawDegrees);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void ZoomPreview(float DeltaDistance);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void ResetPreviewCamera();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void Submit();

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	UDBACharacterCreateZodiacViewModel* GetZodiacStepViewModel() const { return ZodiacStepViewModel; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Element")
	UDBACharacterCreateElementViewModel* GetElementStepViewModel() const { return ElementStepViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|Zodiac")
	FOnZodiacStepError OnZodiacStepError;

	const FDBACharacterCreateDraft* GetDraft() const;
	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
	UDBACharacterCreateDraftSubsystem* GetDraftSubsystem() const;
	UDBAZodiacRegistrySubsystem* GetZodiacRegistry() const;
	UDBACharacterPreviewSubsystem* GetPreviewSubsystem() const;
	UDBASkillGroupGeneratorSubsystem* GetSkillGroupGenerator() const;

protected:
	virtual void BeginDestroy() override;
	void HandleDraftChanged(const FDBACharacterCreateDraft& Draft);
	void HandlePreviewResolved(EDBAZodiac Zodiac, bool bSuccess);
	void HandleSkillGroupDataReady();
	void RefreshAppearanceGroups(const FDBACharacterCreateDraft& Draft);
	void RefreshElementPresentation(const FDBACharacterCreateDraft& Draft);
	void RequestZodiacPresentation(EDBAZodiac Zodiac);
	void ApplyZodiacPresentation(EDBAZodiac Zodiac, UDBAZodiacHeroDataAsset* ZodiacData, uint32 RequestGeneration);
	void PublishZodiacStepError(const FText& Message);
	void UnbindZodiacStep();
	void UnbindElementStep();

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateZodiacViewModel> ZodiacStepViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ElementStepViewModel;

	TWeakObjectPtr<UDBACharacterCreateDraftSubsystem> BoundDraftSubsystem;
	TWeakObjectPtr<UDBACharacterPreviewSubsystem> BoundPreviewSubsystem;
	FDelegateHandle DraftChangedHandle;
	FDelegateHandle PreviewResolvedHandle;
	TWeakObjectPtr<UDBASkillGroupGeneratorSubsystem> BoundSkillGroupGenerator;
	FDelegateHandle SkillGroupDataReadyHandle;
	uint32 ZodiacPresentationRequestGeneration = 0;
	EDBAZodiac LastPreviewZodiac = EDBAZodiac::None;
	FDBACharacterAppearance LastPreviewAppearance;
};
