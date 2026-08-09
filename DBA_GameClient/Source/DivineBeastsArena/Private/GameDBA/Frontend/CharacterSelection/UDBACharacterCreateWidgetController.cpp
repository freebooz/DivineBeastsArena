// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameDBA/Character/Data/DBAZodiacRegistrySubsystem.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupGeneratorSubsystem.h"

UDBACharacterCreateWidgetController::UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterCreateWidgetController::SetCharacterName(const FString& InName)
{
	// Controller 只转发用户意图；名称不在 Widget 或 Controller 中长期保存。
	if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem()) Draft->SetCharacterName(InName);
}

void UDBACharacterCreateWidgetController::SetZodiac(EDBAZodiac InZodiac)
{
	// 保留旧蓝图入口名，但统一走生肖第一步的绑定与异步预览链路。
	SelectZodiac(InZodiac);
}

void UDBACharacterCreateWidgetController::BindZodiacStep()
{
	// 该绑定把三个独立生命周期源收敛到 Controller：Draft 为业务数据、Registry 为静态数据、Preview 为展示状态。
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	UDBAZodiacRegistrySubsystem* Registry = GetZodiacRegistry();
	UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem();
	if (!Draft || !Registry || !Preview)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ZodiacStepUnavailable", "角色创建生肖步骤所需服务尚未就绪。"));
		return;
	}
	if (!ZodiacStepViewModel) ZodiacStepViewModel = NewObject<UDBACharacterCreateZodiacViewModel>(this);
	UnbindZodiacStep();
	BoundDraftSubsystem = Draft;
	BoundPreviewSubsystem = Preview;
	DraftChangedHandle = Draft->OnDraftChanged().AddUObject(this, &UDBACharacterCreateWidgetController::HandleDraftChanged);
	PreviewResolvedHandle = Preview->OnCharacterPreviewResolved.AddUObject(this, &UDBACharacterCreateWidgetController::HandlePreviewResolved);

	TArray<EDBAZodiac> Zodiacs;
	Registry->GetAllZodiacTypes(Zodiacs);
	ZodiacStepViewModel->SetAvailableZodiacs(Zodiacs);
	HandleDraftChanged(Draft->GetDraft());
}

void UDBACharacterCreateWidgetController::BindElementStep()
{
	// 第二步与第一步共享同一个 Draft 变更订阅；不能为元素页再创建独立草稿或独立状态机。
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	UDBASkillGroupGeneratorSubsystem* SkillGroups = GetSkillGroupGenerator();
	if (!Draft || !SkillGroups)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ElementStepUnavailable", "角色创建元素步骤所需配置尚未就绪。"));
		return;
	}
	if (!ElementStepViewModel) ElementStepViewModel = NewObject<UDBACharacterCreateElementViewModel>(this);
	if (!BoundDraftSubsystem.IsValid() || !DraftChangedHandle.IsValid())
	{
		BoundDraftSubsystem = Draft;
		DraftChangedHandle = Draft->OnDraftChanged().AddUObject(this, &UDBACharacterCreateWidgetController::HandleDraftChanged);
	}
	UnbindElementStep();
	BoundSkillGroupGenerator = SkillGroups;
	SkillGroupDataReadyHandle = SkillGroups->OnSkillGroupDataReady.AddUObject(this, &UDBACharacterCreateWidgetController::HandleSkillGroupDataReady);
	HandleDraftChanged(Draft->GetDraft());
}

void UDBACharacterCreateWidgetController::SelectZodiac(const EDBAZodiac InZodiac)
{
	// 先确保绑定，随后只更新草稿；回调会负责异步数据、外观校正和预览刷新。
	if (!ZodiacStepViewModel) BindZodiacStep();
	if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem())
	{
		if (!Draft->SetZodiac(InZodiac)) PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ZodiacInvalid", "所选生肖无效。"));
	}
}

bool UDBACharacterCreateWidgetController::SelectAppearanceOption(const EDBAAppearanceSlot Slot, const FName OptionId)
{
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	if (!Draft || !Draft->SetAppearanceOption(Slot, OptionId))
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "AppearanceInvalid", "该外观选项不适用于当前生肖。"));
		return false;
	}
	return true;
}

bool UDBACharacterCreateWidgetController::ResetAppearance()
{
	FText Reason;
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	if (!Draft || !Draft->ResetAppearance(Reason))
	{
		PublishZodiacStepError(Reason.IsEmpty() ? NSLOCTEXT("DBACharacterCreateController", "AppearanceResetFailed", "外观重置失败。") : Reason);
		return false;
	}
	return true;
}

void UDBACharacterCreateWidgetController::SetElement(EDBAElement InElement)
{
	SelectElement(InElement);
}

void UDBACharacterCreateWidgetController::SelectElement(const EDBAElement InElement)
{
	if (!ElementStepViewModel) BindElementStep();
	if (InElement == EDBAElement::None)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ElementInvalid", "请选择有效元素。"));
		return;
	}

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	UDBASkillGroupGeneratorSubsystem* Generator = GetSkillGroupGenerator();
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	if (!Draft || !Generator || !Generator->GetSkillGroup(Draft->GetDraft().ZodiacType, InElement, SkillGroup)
		|| !SkillGroup.bEnabled || SkillGroup.bIsInDevelopment)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ElementBuildUnavailable", "该元素的固定构筑尚未配置完成，暂不可选择。"));
		return;
	}

	Draft->SetElement(InElement);
}

void UDBACharacterCreateWidgetController::SetFiveCamp(EDBAFiveCamp InFiveCamp)
{
	if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem()) Draft->SetFiveCamp(InFiveCamp);
}

bool UDBACharacterCreateWidgetController::RandomizeAppearance()
{
	const bool bRandomized = GetDraftSubsystem() && GetDraftSubsystem()->RandomizeAppearance();
	if (!bRandomized) PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "AppearanceRandomUnavailable", "外观目录尚未配置或当前生肖没有可随机的合法选项。"));
	return bRandomized;
}

bool UDBACharacterCreateWidgetController::Next()
{
	// 页面切换由 Flow 决定，Controller 不创建、销毁或直接导航任何 Widget。
	return GetLoginFlow() && GetLoginFlow()->AdvanceCharacterCreateDraft();
}

void UDBACharacterCreateWidgetController::Back()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow()) Flow->BackCharacterCreateStep();
}

void UDBACharacterCreateWidgetController::RotatePreview(const float DeltaYawDegrees)
{
	if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem()) Preview->Rotate(DeltaYawDegrees);
}

void UDBACharacterCreateWidgetController::ZoomPreview(const float DeltaDistance)
{
	if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem()) Preview->Zoom(DeltaDistance);
}

void UDBACharacterCreateWidgetController::ResetPreviewCamera()
{
	if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem()) Preview->ResetCamera();
}

void UDBACharacterCreateWidgetController::Submit()
{
	// 兼容旧“创建”按钮：它在前三步执行 Next，在确认页才提交网络创建意图。
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->AdvanceCharacterCreateDraft();
	}
}

const FDBACharacterCreateDraft* UDBACharacterCreateWidgetController::GetDraft() const
{
	return GetDraftSubsystem() ? &GetDraftSubsystem()->GetDraft() : nullptr;
}

UDBAFrontendFlowSubsystem* UDBACharacterCreateWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>()
		: nullptr;
}

UDBACharacterCreateDraftSubsystem* UDBACharacterCreateWidgetController::GetDraftSubsystem() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>()
		: nullptr;
}

UDBAZodiacRegistrySubsystem* UDBACharacterCreateWidgetController::GetZodiacRegistry() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBAZodiacRegistrySubsystem>()
		: nullptr;
}

UDBACharacterPreviewSubsystem* UDBACharacterCreateWidgetController::GetPreviewSubsystem() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>()
		: nullptr;
}

UDBASkillGroupGeneratorSubsystem* UDBACharacterCreateWidgetController::GetSkillGroupGenerator() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBASkillGroupGeneratorSubsystem>()
		: nullptr;
}

void UDBACharacterCreateWidgetController::BeginDestroy()
{
	// Controller 被销毁时解除 Native Delegate，防止已关闭页面继续收到资源加载完成回调。
	UnbindZodiacStep();
	UnbindElementStep();
	Super::BeginDestroy();
}

void UDBACharacterCreateWidgetController::HandleDraftChanged(const FDBACharacterCreateDraft& Draft)
{
	// 两个创建步骤订阅同一 Draft 事件，但各自只刷新自己的显示投影。
	// 因此元素页不会依赖生肖页 Widget 是否仍存活，Back 也不会丢失已选元素。
	if (ElementStepViewModel)
	{
		ElementStepViewModel->ApplyDraft(Draft);
		RefreshElementPresentation(Draft);
	}

	if (!ZodiacStepViewModel) return;
	ZodiacStepViewModel->ApplyDraft(Draft);
	RefreshAppearanceGroups(Draft);
	if (Draft.ZodiacType == EDBAZodiac::None) return;

	// 只在生肖或外观真正变化时发送预览请求，避免名称/元素修改重复加载展示资源。
	if (LastPreviewZodiac != Draft.ZodiacType || LastPreviewAppearance != Draft.Appearance)
	{
		LastPreviewZodiac = Draft.ZodiacType;
		LastPreviewAppearance = Draft.Appearance;
		ZodiacStepViewModel->SetPreviewLoading(true);
		if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem())
		{
			if (!Preview->SelectZodiac(Draft.ZodiacType, Draft.Appearance))
			{
				ZodiacStepViewModel->SetPreviewLoading(false);
				PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "PreviewUnavailable", "角色预览场景尚未准备完成。"));
			}
		}
	}
	RequestZodiacPresentation(Draft.ZodiacType);
}

void UDBACharacterCreateWidgetController::HandleSkillGroupDataReady()
{
	// 主表异步加载完成后仅刷新当前草稿的展示；不触发自动选择元素，
	// 也不调用 GAS，避免资源就绪事件改变玩家已经做出的流程选择。
	if (ElementStepViewModel)
	{
		if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem())
		{
			RefreshElementPresentation(Draft->GetDraft());
		}
	}
}

void UDBACharacterCreateWidgetController::HandlePreviewResolved(const EDBAZodiac Zodiac, const bool bSuccess)
{
	if (!ZodiacStepViewModel || Zodiac != ZodiacStepViewModel->GetSelectedZodiac()) return;
	ZodiacStepViewModel->SetPreviewLoading(false);
	if (!bSuccess) PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "PreviewLoadFailed", "角色预览资源加载失败，请重试或选择其他生肖。"));
}

void UDBACharacterCreateWidgetController::RefreshAppearanceGroups(const FDBACharacterCreateDraft& Draft)
{
	UDBACharacterCreateDraftSubsystem* DraftSubsystem = GetDraftSubsystem();
	if (!DraftSubsystem || !ZodiacStepViewModel) return;
	TArray<FDBAAppearanceOptionGroup> Groups;
	for (uint8 Index = static_cast<uint8>(EDBAAppearanceSlot::Gender); Index <= static_cast<uint8>(EDBAAppearanceSlot::Skin); ++Index)
	{
		const EDBAAppearanceSlot Slot = static_cast<EDBAAppearanceSlot>(Index);
		FDBAAppearanceOptionGroup& Group = Groups.AddDefaulted_GetRef();
		Group.Slot = Slot;
		DraftSubsystem->GetAvailableAppearanceOptionIds(Slot, Group.OptionIds);
		if (Group.OptionIds.IsEmpty())
		{
			Groups.Pop();
			continue;
		}
		Group.SelectedOptionId = Slot == EDBAAppearanceSlot::Equipment
			? (Draft.Appearance.EquipmentVisualIds.IsEmpty() ? NAME_None : Draft.Appearance.EquipmentVisualIds[0])
			: FName();
		if (Slot != EDBAAppearanceSlot::Equipment)
		{
			switch (Slot)
			{
			case EDBAAppearanceSlot::Gender: Group.SelectedOptionId = Draft.Appearance.GenderId; break;
			case EDBAAppearanceSlot::BodyType: Group.SelectedOptionId = Draft.Appearance.BodyTypeId; break;
			case EDBAAppearanceSlot::Face: Group.SelectedOptionId = Draft.Appearance.FaceId; break;
			case EDBAAppearanceSlot::Hair: Group.SelectedOptionId = Draft.Appearance.HairId; break;
			case EDBAAppearanceSlot::HairColor: Group.SelectedOptionId = Draft.Appearance.HairColorId; break;
			case EDBAAppearanceSlot::SkinColor: Group.SelectedOptionId = Draft.Appearance.SkinColorId; break;
			case EDBAAppearanceSlot::EyeColor: Group.SelectedOptionId = Draft.Appearance.EyeColorId; break;
			case EDBAAppearanceSlot::Marking: Group.SelectedOptionId = Draft.Appearance.MarkingId; break;
			case EDBAAppearanceSlot::Horn: Group.SelectedOptionId = Draft.Appearance.HornId; break;
			case EDBAAppearanceSlot::Ear: Group.SelectedOptionId = Draft.Appearance.EarId; break;
			case EDBAAppearanceSlot::Tail: Group.SelectedOptionId = Draft.Appearance.TailId; break;
			case EDBAAppearanceSlot::Weapon: Group.SelectedOptionId = Draft.Appearance.WeaponVisualId; break;
			case EDBAAppearanceSlot::Skin: Group.SelectedOptionId = Draft.Appearance.SkinId; break;
			default: break;
			}
		}
	}
	ZodiacStepViewModel->ApplyAppearanceGroups(Groups);
}

void UDBACharacterCreateWidgetController::RefreshElementPresentation(const FDBACharacterCreateDraft& Draft)
{
	UDBASkillGroupGeneratorSubsystem* Generator = GetSkillGroupGenerator();
	if (!ElementStepViewModel || !Generator) return;

	// 元素集合由固定技能组生成器提供，而非创建页硬编码五个按钮。
	// 表尚未就绪时仍保留配置驱动的枚举顺序，但全部卡片禁用并等待就绪事件刷新。
	TArray<FDBACharacterCreateElementCardModel> Cards;
	for (const EDBAElement Element : Generator->GetAllElementTypes())
	{
		FDBACharacterCreateElementCardModel& Card = Cards.AddDefaulted_GetRef();
		Card.Element = Element;
		Card.DisplayName = StaticEnum<EDBAElement>()->GetDisplayNameTextByValue(static_cast<int64>(Element));

		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (Draft.ZodiacType != EDBAZodiac::None && Generator->GetSkillGroup(Draft.ZodiacType, Element, SkillGroup))
		{
			Card.Description = SkillGroup.Description;
			Card.bIsAvailable = SkillGroup.bEnabled && !SkillGroup.bIsInDevelopment;
		}
		else
		{
			Card.Description = NSLOCTEXT("DBACharacterCreateElement", "BuildNotReady", "固定构筑配置加载中。" );
			Card.bIsAvailable = false;
		}
	}
	ElementStepViewModel->ApplyElementCards(Cards);

	FDBAZodiacElementFixedSkillGroupRow SelectedSkillGroup;
	const bool bHasSelectedBuild = Draft.ZodiacType != EDBAZodiac::None
		&& Draft.ElementType != EDBAElement::None
		&& Generator->GetSkillGroup(Draft.ZodiacType, Draft.ElementType, SelectedSkillGroup)
		&& SelectedSkillGroup.bEnabled && !SelectedSkillGroup.bIsInDevelopment;
	ElementStepViewModel->ApplyFixedSkillBuild(bHasSelectedBuild ? &SelectedSkillGroup : nullptr);

	if (bHasSelectedBuild)
	{
		const FText Summary = FText::Format(
			NSLOCTEXT("DBACharacterCreateElement", "FixedBuildSummary", "固定构筑：{0}\n{1}"),
			SelectedSkillGroup.DisplayName,
			SelectedSkillGroup.Description);
		if (UDBACharacterCreateDraftSubsystem* DraftSubsystem = GetDraftSubsystem())
		{
			DraftSubsystem->SetGeneratedBuildPreview(SelectedSkillGroup.RowId, Summary);
		}
	}
}

void UDBACharacterCreateWidgetController::RequestZodiacPresentation(const EDBAZodiac Zodiac)
{
	UDBAZodiacRegistrySubsystem* Registry = GetZodiacRegistry();
	if (!Registry || Zodiac == EDBAZodiac::None) return;
	const uint32 RequestGeneration = ++ZodiacPresentationRequestGeneration;
	TWeakObjectPtr<UDBACharacterCreateWidgetController> WeakThis(this);
	Registry->LoadAsync(Zodiac, FDBAOnZodiacHeroAssetLoaded::CreateLambda([WeakThis, Zodiac, RequestGeneration](const EDBAZodiac, UDBAZodiacHeroDataAsset* Data)
	{
		if (WeakThis.IsValid()) WeakThis->ApplyZodiacPresentation(Zodiac, Data, RequestGeneration);
	}));
}

void UDBACharacterCreateWidgetController::ApplyZodiacPresentation(const EDBAZodiac Zodiac, UDBAZodiacHeroDataAsset* ZodiacData, const uint32 RequestGeneration)
{
	// Registry 与 Preview 都可能异步完成；只允许最新生肖请求写入 ViewModel，杜绝快速连点乱序。
	if (!ZodiacStepViewModel || RequestGeneration != ZodiacPresentationRequestGeneration || Zodiac != ZodiacStepViewModel->GetSelectedZodiac()) return;
	if (!ZodiacData)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ZodiacDataLoadFailed", "生肖静态配置加载失败。"));
		return;
	}
	ZodiacStepViewModel->ApplySelectedZodiacData(*ZodiacData);
}

void UDBACharacterCreateWidgetController::PublishZodiacStepError(const FText& Message)
{
	if (ZodiacStepViewModel) ZodiacStepViewModel->SetValidationMessage(Message);
	OnZodiacStepError.Broadcast(Message);
}

void UDBACharacterCreateWidgetController::UnbindZodiacStep()
{
	if (BoundDraftSubsystem.IsValid() && DraftChangedHandle.IsValid()) BoundDraftSubsystem->OnDraftChanged().Remove(DraftChangedHandle);
	if (BoundPreviewSubsystem.IsValid() && PreviewResolvedHandle.IsValid()) BoundPreviewSubsystem->OnCharacterPreviewResolved.Remove(PreviewResolvedHandle);
	BoundDraftSubsystem.Reset(); BoundPreviewSubsystem.Reset(); DraftChangedHandle.Reset(); PreviewResolvedHandle.Reset();
	++ZodiacPresentationRequestGeneration;
}

void UDBACharacterCreateWidgetController::UnbindElementStep()
{
	// 固定技能组数据表属于 GameInstance 生命周期；页面销毁后必须取消订阅，
	// 避免异步表加载完成时回调到已销毁的创建页 Controller。
	if (BoundSkillGroupGenerator.IsValid() && SkillGroupDataReadyHandle.IsValid())
	{
		BoundSkillGroupGenerator->OnSkillGroupDataReady.Remove(SkillGroupDataReadyHandle);
	}
	BoundSkillGroupGenerator.Reset();
	SkillGroupDataReadyHandle.Reset();
}
