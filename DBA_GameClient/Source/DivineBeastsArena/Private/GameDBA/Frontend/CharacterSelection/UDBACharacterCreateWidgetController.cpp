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
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmViewModel.h"
#include "GameDBA/Character/Data/DBAZodiacRegistrySubsystem.h"
#include "GameDBA/Data/Tables/DBAFiveCampDisplayData.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "GameDBA/Frontend/Preview/DBAFiveCampPreviewTheme.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupGeneratorSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"

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
	SelectFiveCamp(InFiveCamp);
}

void UDBACharacterCreateWidgetController::BindFiveCampStep()
{
	// 五营页面与前两步复用同一个 Draft 订阅，避免为第三步再创建互相竞争的创建状态机。
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	if (!Draft)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampDraftUnavailable", "角色创建五营步骤所需草稿服务尚未就绪。"));
		return;
	}
	if (!FiveCampStepViewModel)
	{
		FiveCampStepViewModel = NewObject<UDBACharacterCreateFiveCampViewModel>(this);
	}
	if (!BoundDraftSubsystem.IsValid() || !DraftChangedHandle.IsValid())
	{
		BoundDraftSubsystem = Draft;
		DraftChangedHandle = Draft->OnDraftChanged().AddUObject(this, &UDBACharacterCreateWidgetController::HandleDraftChanged);
	}
	RequestFiveCampDisplayTable();
	HandleDraftChanged(Draft->GetDraft());
}

void UDBACharacterCreateWidgetController::SelectFiveCamp(const EDBAFiveCamp InFiveCamp)
{
	if (!FiveCampStepViewModel)
	{
		BindFiveCampStep();
	}
	if (InFiveCamp == EDBAFiveCamp::None || !FiveCampStepViewModel)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampInvalid", "请选择有效的五营主题。"));
		return;
	}

	const FDBACharacterCreateFiveCampCardModel* SelectedCard = nullptr;
	for (const FDBACharacterCreateFiveCampCardModel& Card : FiveCampStepViewModel->GetFiveCampCards())
	{
		if (Card.FiveCamp == InFiveCamp)
		{
			SelectedCard = &Card;
			break;
		}
	}
	if (!SelectedCard || !SelectedCard->bIsAvailable)
	{
		PublishZodiacStepError(SelectedCard && !SelectedCard->UnavailableReason.IsEmpty()
			? SelectedCard->UnavailableReason
			: NSLOCTEXT("DBACharacterCreateController", "FiveCampNotConfigured", "该五营主题尚未配置或当前不可用。"));
		return;
	}

	if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem())
	{
		// Draft 只保存 EDBAFiveCamp；这里不引用任何 TeamId/Faction 类型，彻底隔离账号创建与对局敌我关系。
		Draft->SetFiveCamp(InFiveCamp);
	}
}

void UDBACharacterCreateWidgetController::BindConfirmStep()
{
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	UDBAFrontendFlowSubsystem* Flow = GetLoginFlow();
	if (!Draft || !Flow)
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "ConfirmUnavailable", "角色创建确认步骤所需服务尚未就绪。"));
		return;
	}
	if (!ConfirmStepViewModel)
	{
		ConfirmStepViewModel = NewObject<UDBACharacterCreateConfirmViewModel>(this);
	}
	if (!BoundDraftSubsystem.IsValid() || !DraftChangedHandle.IsValid())
	{
		BoundDraftSubsystem = Draft;
		DraftChangedHandle = Draft->OnDraftChanged().AddUObject(this, &UDBACharacterCreateWidgetController::HandleDraftChanged);
	}
	UnbindConfirmStep();
	BoundConfirmFlow = Flow;
	Flow->OnCharacterCreateCompleted.AddDynamic(this, &UDBACharacterCreateWidgetController::HandleCharacterCreateCompleted);
	ConfirmStepViewModel->ApplyDraft(Draft->GetDraft());
}

bool UDBACharacterCreateWidgetController::SubmitConfirmedCharacterCreation()
{
	if (!ConfirmStepViewModel)
	{
		BindConfirmStep();
	}
	UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem();
	UDBAFrontendFlowSubsystem* Flow = GetLoginFlow();
	if (!Draft || !Flow || !ConfirmStepViewModel)
	{
		return false;
	}
	FText Reason;
	FDBACharacterCreateRequest Request;
	if (!Draft->BuildCreateRequest(Request, Reason))
	{
		FDBAApiError Error = UDBAFrontendErrorMapper::FromCharacterCreateErrorCode(TEXT("INVALID_NAME"));
		Error.UserMessage = Reason.IsEmpty() ? NSLOCTEXT("DBACharacterCreateController", "ConfirmDraftInvalid", "请检查角色名与创建选项。") : Reason;
		ConfirmStepViewModel->SetError(Error);
		return false;
	}

	ConfirmStepViewModel->ClearError();
	ConfirmStepViewModel->SetSubmitting(true);
	// Flow 忽略临时 Request 内容而重新从 Draft 构建，避免 Widget 通过参数绕过统一校验与幂等门闩。
	Flow->SubmitCharacterCreation(Request);
	return true;
}

void UDBACharacterCreateWidgetController::CancelConfirmedCharacterCreation()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->CancelCharacterCreationSubmission();
	}
	if (ConfirmStepViewModel)
	{
		ConfirmStepViewModel->SetSubmitting(false);
	}
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
		if (Flow->GetFrontendState() == EDBAFrontendState::CharacterCreate_Confirm)
		{
			SubmitConfirmedCharacterCreation();
			return;
		}
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
	UnbindFiveCampStep();
	UnbindConfirmStep();
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

	if (FiveCampStepViewModel)
	{
		FiveCampStepViewModel->ApplyDraft(Draft);
		RefreshFiveCampPresentation(Draft);
	}

	if (ConfirmStepViewModel)
	{
		ConfirmStepViewModel->ApplyDraft(Draft);
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

void UDBACharacterCreateWidgetController::HandleCharacterCreateCompleted(const FDBAOperationResult& Result, const FDBACharacterSummary& Character)
{
	if (!ConfirmStepViewModel)
	{
		return;
	}
	ConfirmStepViewModel->SetSubmitting(false);
	if (Result.bSuccess)
	{
		ConfirmStepViewModel->ClearError();
		return;
	}
	FDBAApiError Error = Result.ApiError;
	if (!Error.IsError())
	{
		Error = UDBAFrontendErrorMapper::FromLegacyMessage(Result.ErrorMessage.IsEmpty() ? TEXT("角色创建失败。") : Result.ErrorMessage);
	}
	ConfirmStepViewModel->SetError(Error);
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

void UDBACharacterCreateWidgetController::RequestFiveCampDisplayTable()
{
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (!Settings || Settings->CharacterCreateFiveCampDisplayTable.IsNull())
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampTableMissing", "尚未在 DBA Frontend 设置中配置角色创建五营显示数据表。"));
		return;
	}

	const uint32 RequestGeneration = ++FiveCampDisplayTableRequestGeneration;
	if (FiveCampDisplayTableLoadHandle.IsValid())
	{
		FiveCampDisplayTableLoadHandle->CancelHandle();
		FiveCampDisplayTableLoadHandle.Reset();
	}

	TWeakObjectPtr<UDBACharacterCreateWidgetController> WeakThis(this);
	FiveCampDisplayTableLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Settings->CharacterCreateFiveCampDisplayTable.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda([WeakThis, RequestGeneration, Table = Settings->CharacterCreateFiveCampDisplayTable]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ApplyFiveCampDisplayTable(Table.Get(), RequestGeneration);
			}
		}));
	if (!FiveCampDisplayTableLoadHandle.IsValid())
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampTableLoadRequestFailed", "五营显示数据表异步加载请求无法启动。"));
	}
}

void UDBACharacterCreateWidgetController::ApplyFiveCampDisplayTable(UDataTable* DisplayTable, const uint32 RequestGeneration)
{
	// 数据表回调可能在页面已经关闭、配置已经切换后才到达；仅接受最新请求，防止旧主题覆盖新页面。
	if (RequestGeneration != FiveCampDisplayTableRequestGeneration || !FiveCampStepViewModel)
	{
		return;
	}
	FiveCampDisplayTableLoadHandle.Reset();
	if (!DisplayTable || DisplayTable->GetRowStruct() != FDBAFiveCampDisplayRow::StaticStruct())
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampTableInvalid", "五营显示数据表缺失或行结构不是 FDBAFiveCampDisplayRow。"));
		return;
	}

	TArray<FDBACharacterCreateFiveCampCardModel> Cards;
	TSet<EDBAFiveCamp> SeenCamps;
	for (const TPair<FName, uint8*>& Pair : DisplayTable->GetRowMap())
	{
		const FDBAFiveCampDisplayRow* Row = reinterpret_cast<const FDBAFiveCampDisplayRow*>(Pair.Value);
		if (!Row)
		{
			continue;
		}
		const EDBAFiveCamp FiveCamp = static_cast<EDBAFiveCamp>(Row->FiveCampEnum);
		if (FiveCamp == EDBAFiveCamp::None || !StaticEnum<EDBAFiveCamp>()->IsValidEnumValue(static_cast<int64>(FiveCamp)))
		{
			UE_LOG(LogDBAFrontend, Error, TEXT("[角色创建] 五营显示表行 %s 使用了无效的 FiveCampEnum=%d。"), *Pair.Key.ToString(), Row->FiveCampEnum);
			continue;
		}
		if (SeenCamps.Contains(FiveCamp))
		{
			UE_LOG(LogDBAFrontend, Error, TEXT("[角色创建] 五营显示表存在重复的五营枚举：%d，重复行=%s。"), static_cast<int32>(FiveCamp), *Pair.Key.ToString());
			continue;
		}
		SeenCamps.Add(FiveCamp);

		FDBACharacterCreateFiveCampCardModel& Card = Cards.AddDefaulted_GetRef();
		Card.FiveCamp = FiveCamp;
		Card.SourceRowName = Pair.Key;
		Card.DisplayName = !Row->DisplayNameCN.IsEmpty()
			? Row->DisplayNameCN
			: StaticEnum<EDBAFiveCamp>()->GetDisplayNameTextByValue(static_cast<int64>(FiveCamp));
		Card.Description = Row->Description;
		Card.Icon = Row->IconTexture;
		Card.Emblem = Row->EmblemTexture;
		Card.ThemeColor = Row->ThemeColor;
		Card.SecondaryColor = Row->SecondaryColor;
		Card.bIsAvailable = Row->bIsAvailable && Row->UnlockLevel <= 0;
		if (!Row->bIsAvailable)
		{
			Card.UnavailableReason = NSLOCTEXT("DBACharacterCreateController", "FiveCampDisabled", "该五营主题暂未开放。");
		}
		else if (Row->UnlockLevel > 0)
		{
			Card.UnavailableReason = FText::Format(
				NSLOCTEXT("DBACharacterCreateController", "FiveCampLocked", "该五营主题需要角色等级 {0} 才能选择。"),
				FText::AsNumber(Row->UnlockLevel));
		}
	}

	Cards.Sort([](const FDBACharacterCreateFiveCampCardModel& Left, const FDBACharacterCreateFiveCampCardModel& Right)
	{
		return static_cast<uint8>(Left.FiveCamp) < static_cast<uint8>(Right.FiveCamp);
	});
	FiveCampStepViewModel->ApplyFiveCampCards(Cards);
	if (Cards.IsEmpty())
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampRowsUnavailable", "五营显示数据表没有可用的有效行。"));
		return;
	}
	if (UDBACharacterCreateDraftSubsystem* Draft = GetDraftSubsystem())
	{
		RefreshFiveCampPresentation(Draft->GetDraft());
	}
}

void UDBACharacterCreateWidgetController::RefreshFiveCampPresentation(const FDBACharacterCreateDraft& Draft)
{
	if (!FiveCampStepViewModel)
	{
		return;
	}
	if (Draft.FiveCampType == EDBAFiveCamp::None)
	{
		if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem())
		{
			Preview->ClearFiveCampTheme();
		}
		return;
	}

	const FDBACharacterCreateFiveCampCardModel* Card = nullptr;
	for (const FDBACharacterCreateFiveCampCardModel& Candidate : FiveCampStepViewModel->GetFiveCampCards())
	{
		if (Candidate.FiveCamp == Draft.FiveCampType)
		{
			Card = &Candidate;
			break;
		}
	}
	if (!Card || !Card->bIsAvailable)
	{
		return;
	}

	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	UDataTable* DisplayTable = Settings ? Settings->CharacterCreateFiveCampDisplayTable.Get() : nullptr;
	const FDBAFiveCampDisplayRow* Row = DisplayTable ? DisplayTable->FindRow<FDBAFiveCampDisplayRow>(Card->SourceRowName, TEXT("角色创建五营主题"), false) : nullptr;
	if (!Row)
	{
		return;
	}

	// 表行的大资源仍保持软引用；在此只加载当前已选择主题，且由 PreviewStage 接收已解析对象。
	TArray<FSoftObjectPath> ThemeAssets;
	if (!Row->BackgroundTexture.IsNull()) { ThemeAssets.Add(Row->BackgroundTexture.ToSoftObjectPath()); }
	if (!Row->EmblemTexture.IsNull()) { ThemeAssets.Add(Row->EmblemTexture.ToSoftObjectPath()); }
	if (!Row->EffectMaterial.IsNull()) { ThemeAssets.Add(Row->EffectMaterial.ToSoftObjectPath()); }
	if (!Row->ThemeSound.IsNull()) { ThemeAssets.Add(Row->ThemeSound.ToSoftObjectPath()); }
	const uint32 ThemeRequestGeneration = ++FiveCampThemeRequestGeneration;
	TWeakObjectPtr<UDBACharacterCreateWidgetController> WeakThis(this);
	const auto ApplyResolvedTheme = [WeakThis, ThemeRequestGeneration, FiveCamp = Draft.FiveCampType, RowName = Card->SourceRowName]()
	{
		if (!WeakThis.IsValid() || ThemeRequestGeneration != WeakThis->FiveCampThemeRequestGeneration)
		{
			return;
		}
		const UDBAFrontendSettings* CurrentSettings = GetDefault<UDBAFrontendSettings>();
		UDataTable* CurrentTable = CurrentSettings ? CurrentSettings->CharacterCreateFiveCampDisplayTable.Get() : nullptr;
		const FDBAFiveCampDisplayRow* CurrentRow = CurrentTable ? CurrentTable->FindRow<FDBAFiveCampDisplayRow>(RowName, TEXT("角色创建五营主题"), false) : nullptr;
		UDBACharacterCreateDraftSubsystem* CurrentDraft = WeakThis->GetDraftSubsystem();
		if (!CurrentRow || !CurrentDraft || CurrentDraft->GetDraft().FiveCampType != FiveCamp)
		{
			return;
		}
		FDBAFiveCampPreviewTheme Theme;
		Theme.FiveCamp = FiveCamp;
		Theme.DisplayName = CurrentRow->DisplayNameCN;
		Theme.ThemeColor = CurrentRow->ThemeColor;
		Theme.SecondaryColor = CurrentRow->SecondaryColor;
		Theme.BackgroundTexture = CurrentRow->BackgroundTexture.Get();
		Theme.EmblemTexture = CurrentRow->EmblemTexture.Get();
		Theme.VfxMaterial = CurrentRow->EffectMaterial.Get();
		Theme.ThemeSound = CurrentRow->ThemeSound.Get();
		if (UDBACharacterPreviewSubsystem* Preview = WeakThis->GetPreviewSubsystem())
		{
			Preview->ApplyFiveCampTheme(Theme);
		}
	};
	if (FiveCampThemeLoadHandle.IsValid())
	{
		FiveCampThemeLoadHandle->CancelHandle();
		FiveCampThemeLoadHandle.Reset();
	}
	if (ThemeAssets.IsEmpty())
	{
		// 没有可选大资源时也要更新主题色投影，不能因空资源数组让选中态停留在旧主题。
		ApplyResolvedTheme();
		return;
	}
	FiveCampThemeLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(ThemeAssets, FStreamableDelegate::CreateLambda(ApplyResolvedTheme));
	if (!FiveCampThemeLoadHandle.IsValid())
	{
		PublishZodiacStepError(NSLOCTEXT("DBACharacterCreateController", "FiveCampThemeLoadRequestFailed", "五营主题资源异步加载请求无法启动。"));
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
	// 该历史名称兼容第一步事件；第二步复用同一结构化中文错误出口，
	// 避免 Element Widget 根据英文消息或底层 DataTable 错误自行判断状态。
	if (ZodiacStepViewModel) ZodiacStepViewModel->SetValidationMessage(Message);
	if (ElementStepViewModel) ElementStepViewModel->SetValidationMessage(Message);
	if (FiveCampStepViewModel) FiveCampStepViewModel->SetValidationMessage(Message);
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

void UDBACharacterCreateWidgetController::UnbindFiveCampStep()
{
	// 取消页面私有异步加载，防止关闭创建页后旧主题回调写入新的前台 Screen。
	++FiveCampDisplayTableRequestGeneration;
	if (FiveCampDisplayTableLoadHandle.IsValid())
	{
		FiveCampDisplayTableLoadHandle->CancelHandle();
		FiveCampDisplayTableLoadHandle.Reset();
	}
	++FiveCampThemeRequestGeneration;
	if (FiveCampThemeLoadHandle.IsValid())
	{
		FiveCampThemeLoadHandle->CancelHandle();
		FiveCampThemeLoadHandle.Reset();
	}
	if (UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem())
	{
		Preview->ClearFiveCampTheme();
	}
}

void UDBACharacterCreateWidgetController::UnbindConfirmStep()
{
	if (BoundConfirmFlow.IsValid())
	{
		BoundConfirmFlow->OnCharacterCreateCompleted.RemoveDynamic(this, &UDBACharacterCreateWidgetController::HandleCharacterCreateCompleted);
	}
	BoundConfirmFlow.Reset();
}
