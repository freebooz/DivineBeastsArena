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
#include "GameDBA/Core/DBAResultTypes.h"
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
class UDBACharacterCreateFiveCampViewModel;
class UDBACharacterCreateConfirmViewModel;
class UDBAZodiacHeroDataAsset;
class UDataTable;
struct FStreamableHandle;
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

	/**
	 * 绑定角色创建第三步的五营数据表与 Draft。表资源按需异步加载，重复调用不会建立第二套业务状态；
	 * 五营仅影响 PreviewStage 主题，不会触及 TeamId、旧 Faction 或正式角色外观组件。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	void BindFiveCampStep();

	/** 仅接受已由数据表声明且可用的五营选择意图，再写入唯一 CharacterCreateDraft。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	void SelectFiveCamp(EDBAFiveCamp InFiveCamp);

	/** 绑定确认页投影及创建完成事件；所有提交仍由 Flow 委托 Roster，Widget 不接触 ApiClient。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void BindConfirmStep();

	/** 本地仅做 Draft 的快速校验，成功后交由 Flow -> CharacterRosterSubsystem 发送服务端权威创建请求。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	bool SubmitConfirmedCharacterCreation();

	/** 取消本地等待并保留 Draft/幂等键；不会假定服务端一定未收到请求。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void CancelConfirmedCharacterCreation();

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

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FiveCamp")
	UDBACharacterCreateFiveCampViewModel* GetFiveCampStepViewModel() const { return FiveCampStepViewModel; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	UDBACharacterCreateConfirmViewModel* GetConfirmStepViewModel() const { return ConfirmStepViewModel; }

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
	UFUNCTION()
	void HandleCharacterCreateCompleted(const FDBAOperationResult& Result, const FDBACharacterSummary& Character);
	void RefreshAppearanceGroups(const FDBACharacterCreateDraft& Draft);
	void RefreshElementPresentation(const FDBACharacterCreateDraft& Draft);
	void RefreshFiveCampPresentation(const FDBACharacterCreateDraft& Draft);
	void RequestFiveCampDisplayTable();
	void ApplyFiveCampDisplayTable(UDataTable* DisplayTable, uint32 RequestGeneration);
	void RequestZodiacPresentation(EDBAZodiac Zodiac);
	void ApplyZodiacPresentation(EDBAZodiac Zodiac, UDBAZodiacHeroDataAsset* ZodiacData, uint32 RequestGeneration);
	void PublishZodiacStepError(const FText& Message);
	void UnbindZodiacStep();
	void UnbindElementStep();
	void UnbindFiveCampStep();
	void UnbindConfirmStep();

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateZodiacViewModel> ZodiacStepViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ElementStepViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateFiveCampViewModel> FiveCampStepViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateConfirmViewModel> ConfirmStepViewModel;

	TWeakObjectPtr<UDBACharacterCreateDraftSubsystem> BoundDraftSubsystem;
	TWeakObjectPtr<UDBACharacterPreviewSubsystem> BoundPreviewSubsystem;
	FDelegateHandle DraftChangedHandle;
	FDelegateHandle PreviewResolvedHandle;
	TWeakObjectPtr<UDBASkillGroupGeneratorSubsystem> BoundSkillGroupGenerator;
	TWeakObjectPtr<UDBAFrontendFlowSubsystem> BoundConfirmFlow;
	FDelegateHandle SkillGroupDataReadyHandle;
	TSharedPtr<FStreamableHandle> FiveCampDisplayTableLoadHandle;
	uint32 FiveCampDisplayTableRequestGeneration = 0;
	TSharedPtr<FStreamableHandle> FiveCampThemeLoadHandle;
	uint32 FiveCampThemeRequestGeneration = 0;
	uint32 ZodiacPresentationRequestGeneration = 0;
	EDBAZodiac LastPreviewZodiac = EDBAZodiac::None;
	FDBACharacterAppearance LastPreviewAppearance;
};
