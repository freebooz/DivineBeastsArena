// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameDBA/Character/Appearance/DBAAppearanceCatalogDataAsset.h"
#include "GameDBA/Character/Data/DBAZodiacRegistrySubsystem.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	/** 将目录枚举槽位映射到草稿中的稳定 ID 字段；这里不接触任何资源路径。 */
	void SetAppearanceSlotValue(FDBACharacterAppearance& Appearance, const EDBAAppearanceSlot Slot, const FName Value)
	{
		switch (Slot)
		{
		case EDBAAppearanceSlot::Gender: Appearance.GenderId = Value; break;
		case EDBAAppearanceSlot::BodyType: Appearance.BodyTypeId = Value; break;
		case EDBAAppearanceSlot::Face: Appearance.FaceId = Value; break;
		case EDBAAppearanceSlot::Hair: Appearance.HairId = Value; break;
		case EDBAAppearanceSlot::HairColor: Appearance.HairColorId = Value; break;
		case EDBAAppearanceSlot::SkinColor: Appearance.SkinColorId = Value; break;
		case EDBAAppearanceSlot::EyeColor: Appearance.EyeColorId = Value; break;
		case EDBAAppearanceSlot::Marking: Appearance.MarkingId = Value; break;
		case EDBAAppearanceSlot::Horn: Appearance.HornId = Value; break;
		case EDBAAppearanceSlot::Ear: Appearance.EarId = Value; break;
		case EDBAAppearanceSlot::Tail: Appearance.TailId = Value; break;
		case EDBAAppearanceSlot::Weapon: Appearance.WeaponVisualId = Value; break;
		case EDBAAppearanceSlot::Skin: Appearance.SkinId = Value; break;
		default: break;
		}
	}

	/** 读取草稿槽位的当前稳定 ID；Equipment 作为多选槽位由调用方单独处理。 */
	FName GetAppearanceSlotValue(const FDBACharacterAppearance& Appearance, const EDBAAppearanceSlot Slot)
	{
		switch (Slot)
		{
		case EDBAAppearanceSlot::Gender: return Appearance.GenderId;
		case EDBAAppearanceSlot::BodyType: return Appearance.BodyTypeId;
		case EDBAAppearanceSlot::Face: return Appearance.FaceId;
		case EDBAAppearanceSlot::Hair: return Appearance.HairId;
		case EDBAAppearanceSlot::HairColor: return Appearance.HairColorId;
		case EDBAAppearanceSlot::SkinColor: return Appearance.SkinColorId;
		case EDBAAppearanceSlot::EyeColor: return Appearance.EyeColorId;
		case EDBAAppearanceSlot::Marking: return Appearance.MarkingId;
		case EDBAAppearanceSlot::Horn: return Appearance.HornId;
		case EDBAAppearanceSlot::Ear: return Appearance.EarId;
		case EDBAAppearanceSlot::Tail: return Appearance.TailId;
		case EDBAAppearanceSlot::Weapon: return Appearance.WeaponVisualId;
		case EDBAAppearanceSlot::Skin: return Appearance.SkinId;
		default: return NAME_None;
		}
	}

	/** DataAsset 使用枚举名称作为键；仅接受合法槽位，未知键会被安全忽略。 */
	void ApplyDefaultOption(FDBACharacterAppearance& Appearance, const FName SlotName, const FName OptionId)
	{
		const UEnum* SlotEnum = StaticEnum<EDBAAppearanceSlot>();
		const int64 Value = SlotEnum ? SlotEnum->GetValueByName(SlotName) : INDEX_NONE;
		if (Value != INDEX_NONE)
		{
			SetAppearanceSlotValue(Appearance, static_cast<EDBAAppearanceSlot>(Value), OptionId);
		}
	}

	/** 恢复本地草稿时将机器可读枚举名称还原为数值，避免依赖本地化显示文本。 */
	bool ParseEnumValue(const UEnum* Enum, const FString& Value, int64& OutValue)
	{
		OutValue = INDEX_NONE;
		if (!Enum)
		{
			return false;
		}
		OutValue = Enum->GetValueByNameString(Value);
		return OutValue != INDEX_NONE;
	}
}

void UDBACharacterCreateDraftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 注册表只提供当前生肖的按需加载，不会在启动阶段同步加载十二生肖展示资源。
	Collection.InitializeDependency<UDBAZodiacRegistrySubsystem>();
	Super::Initialize(Collection);
	BeginDraft();
	RequestAppearanceCatalogAsync();
}

void UDBACharacterCreateDraftSubsystem::Deinitialize()
{
	if (AppearanceCatalogLoadHandle.IsValid())
	{
		AppearanceCatalogLoadHandle->CancelHandle();
		AppearanceCatalogLoadHandle.Reset();
	}
	AppearanceCatalog.Reset();
	AllowedAppearanceOptionIds.Reset();
	DefaultAppearanceOptionIds.Reset();
	Super::Deinitialize();
}

bool UDBACharacterCreateDraftSubsystem::IsSupportedInCurrentEnvironment() const
{
	return !IsRunningDedicatedServer();
}

void UDBACharacterCreateDraftSubsystem::BeginDraft()
{
	// 每次进入创建链都必须获得一份干净草稿，防止上次取消的名称或外观泄漏到新角色。
	Draft = FDBACharacterCreateDraft();
	++DraftRequestVersion;
	AllowedAppearanceOptionIds.Reset();
	BroadcastDraftChanged();
}

void UDBACharacterCreateDraftSubsystem::ResetDraft()
{
	BeginDraft();
}

bool UDBACharacterCreateDraftSubsystem::SetZodiac(const EDBAZodiac Zodiac)
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}
	// 更换生肖后保留原外观到异步数据到达，再显式 Normalize：可复用的 ID 会保留，非法 ID 会被替换或清空。
	Draft.ZodiacType = Zodiac;
	AllowedAppearanceOptionIds.Reset();
	DefaultAppearanceOptionIds.Reset();
	const uint32 RequestVersion = ++DraftRequestVersion;
	RequestZodiacDefaultsAsync(Zodiac, RequestVersion);
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SetAppearance(const FDBACharacterAppearance& Appearance)
{
	// 先临时写入后验证；失败时恢复原值，保证调用者不能留下半合法的草稿状态。
	const FDBACharacterAppearance PreviousAppearance = Draft.Appearance;
	Draft.Appearance = Appearance;
	FText Reason;
	if (!ValidateAppearance(Reason))
	{
		Draft.Appearance = PreviousAppearance;
		UE_LOG(LogDBACharacter, Warning, TEXT("角色创建草稿拒绝非法外观：%s"), *Reason.ToString());
		return false;
	}
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SetAppearanceOption(const EDBAAppearanceSlot Slot, const FName OptionId)
{
	FDBACharacterAppearance Candidate = Draft.Appearance;
	if (Slot == EDBAAppearanceSlot::Equipment)
	{
		Candidate.EquipmentVisualIds = OptionId.IsNone() ? TArray<FName>() : TArray<FName>({ OptionId });
	}
	else
	{
		SetAppearanceSlotValue(Candidate, Slot, OptionId);
	}
	return SetAppearance(Candidate);
}

bool UDBACharacterCreateDraftSubsystem::ResetAppearance(FText& OutReason)
{
	if (Draft.ZodiacType == EDBAZodiac::None)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "ResetAppearanceNeedZodiac", "请先选择生肖后再重置外观。");
		return false;
	}
	Draft.Appearance = FDBACharacterAppearance();
	for (const TPair<FName, FName>& Pair : DefaultAppearanceOptionIds)
	{
		ApplyDefaultOption(Draft.Appearance, Pair.Key, Pair.Value);
	}
	if (!NormalizeAppearance(OutReason))
	{
		return false;
	}
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::NormalizeAppearance(FText& OutReason)
{
	OutReason = FText::GetEmpty();
	if (Draft.ZodiacType == EDBAZodiac::None)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "NormalizeAppearanceNeedZodiac", "未选择生肖，无法校正外观。");
		return false;
	}

	FDBACharacterAppearance Candidate = Draft.Appearance;
	bool bChanged = false;
	for (uint8 Index = static_cast<uint8>(EDBAAppearanceSlot::Gender); Index <= static_cast<uint8>(EDBAAppearanceSlot::Skin); ++Index)
	{
		const EDBAAppearanceSlot Slot = static_cast<EDBAAppearanceSlot>(Index);
		if (Slot == EDBAAppearanceSlot::Equipment) continue;
		const FName Current = GetAppearanceSlotValue(Candidate, Slot);
		if (Current.IsNone() || IsAppearanceOptionAllowed(Current)) continue;

		FName Replacement = NAME_None;
		const FName SlotName = StaticEnum<EDBAAppearanceSlot>()->GetNameByValue(static_cast<int64>(Slot));
		if (const FName* Default = DefaultAppearanceOptionIds.Find(SlotName); Default && IsAppearanceOptionAllowed(*Default))
		{
			Replacement = *Default;
		}
		else if (const UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get())
		{
			if (const FDBAAppearanceOptionDefinition* Fallback = Catalog->FindFallback(Slot, Draft.ZodiacType))
			{
				Replacement = Fallback->OptionId;
			}
		}
		SetAppearanceSlotValue(Candidate, Slot, Replacement);
		bChanged = true;
	}

	Candidate.EquipmentVisualIds.RemoveAll([this](const FName OptionId) { return !IsAppearanceOptionAllowed(OptionId); });
	if (Candidate != Draft.Appearance)
	{
		Draft.Appearance = Candidate;
		bChanged = true;
	}
	if (bChanged)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "AppearanceNormalized", "已按当前生肖校正不兼容的外观选项。");
		BroadcastDraftChanged();
	}
	// 成功归一化同样需要保留其中文说明，不能被后续成功校验覆盖为空，
	// 否则界面无法明确提示玩家：切换生肖后哪些旧外观已被安全回退。
	FText ValidationReason;
	if (!ValidateAppearance(ValidationReason))
	{
		OutReason = ValidationReason;
		return false;
	}

	return true;
}

void UDBACharacterCreateDraftSubsystem::GetAvailableAppearanceOptionIds(const EDBAAppearanceSlot Slot, TArray<FName>& OutOptionIds) const
{
	OutOptionIds.Reset();
	const UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get();
	if (!Catalog || Draft.ZodiacType == EDBAZodiac::None) return;
	Catalog->GetAvailableOptionIds(Draft.ZodiacType, Slot, OutOptionIds);
	OutOptionIds.RemoveAll([this](const FName OptionId) { return !AllowedAppearanceOptionIds.IsEmpty() && !AllowedAppearanceOptionIds.Contains(OptionId); });
}

bool UDBACharacterCreateDraftSubsystem::RandomizeAppearance()
{
	// 未加载目录时拒绝随机，而不是猜测资源或生成服务端无法验证的 OptionId。
	if (Draft.ZodiacType == EDBAZodiac::None || !AppearanceCatalog.IsValid())
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("角色创建草稿无法随机外观：生肖或外观目录尚未准备完成。"));
		return false;
	}

	FDBACharacterAppearance Candidate = Draft.Appearance;
	// 逐槽位从目录候选项中挑选；RandomizeAppearanceSlot 会二次过滤生肖白名单。
	for (uint8 Index = static_cast<uint8>(EDBAAppearanceSlot::Gender); Index <= static_cast<uint8>(EDBAAppearanceSlot::Skin); ++Index)
	{
		RandomizeAppearanceSlot(static_cast<EDBAAppearanceSlot>(Index), Candidate);
	}

	return SetAppearance(Candidate);
}

bool UDBACharacterCreateDraftSubsystem::SetGeneratedBuildPreview(const FName InFixedSkillBuildRowId, const FText& InPreviewSummary)
{
	// 构筑摘要只由固定技能组查询结果写入。它用于创建页显示，不参与客户端权威授予，
	// 因此在生肖或元素尚未完成时拒绝保留任何可能过期的技能组身份。
	if (Draft.ZodiacType == EDBAZodiac::None || Draft.ElementType == EDBAElement::None || InFixedSkillBuildRowId.IsNone())
	{
		return false;
	}

	if (Draft.FixedSkillBuildRowId == InFixedSkillBuildRowId && Draft.PreviewSummary.EqualTo(InPreviewSummary))
	{
		return true;
	}

	Draft.FixedSkillBuildRowId = InFixedSkillBuildRowId;
	Draft.PreviewSummary = InPreviewSummary;
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SetElement(const EDBAElement Element)
{
	if (Element == EDBAElement::None)
	{
		return false;
	}
	Draft.ElementType = Element;
	// 元素变更后旧构筑不再与当前身份匹配；必须先清空，再等待规则子系统生成新的只读摘要。
	Draft.FixedSkillBuildRowId = NAME_None;
	Draft.PreviewSummary = FText::GetEmpty();
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SetFiveCamp(const EDBAFiveCamp FiveCamp)
{
	if (FiveCamp == EDBAFiveCamp::None)
	{
		return false;
	}
	Draft.FiveCampType = FiveCamp;
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SetCharacterName(const FString& Name)
{
	Draft.CharacterName = Name.TrimStartAndEnd();
	BroadcastDraftChanged();
	return !Draft.CharacterName.IsEmpty();
}

bool UDBACharacterCreateDraftSubsystem::CanEnter(const EDBACharacterCreateStep Step, FText& OutReason) const
{
	// 该守卫只描述账号创建链的前置条件，绝不能复用赛前选人倒计时或对局状态。
	OutReason = FText::GetEmpty();
	switch (Step)
	{
	case EDBACharacterCreateStep::ZodiacAppearance: return true;
	case EDBACharacterCreateStep::Element:
		if (Draft.ZodiacType != EDBAZodiac::None) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "NeedZodiac", "请先选择生肖。"); return false;
	case EDBACharacterCreateStep::FiveCamp:
		if (Draft.ZodiacType != EDBAZodiac::None && Draft.ElementType != EDBAElement::None) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "NeedElement", "请先完成生肖和元素选择。"); return false;
	case EDBACharacterCreateStep::ConfirmName:
		if (Draft.ZodiacType != EDBAZodiac::None && Draft.ElementType != EDBAElement::None && Draft.FiveCampType != EDBAFiveCamp::None) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "NeedFiveCamp", "请先完成生肖、元素和五营选择。"); return false;
	default: OutReason = NSLOCTEXT("DBACharacterCreateDraft", "UnknownStep", "角色创建步骤无效。"); return false;
	}
}

bool UDBACharacterCreateDraftSubsystem::CanLeave(FText& OutReason) const
{
	// 离开当前步骤的校验比进入校验严格：必须确保本步骤自身已完成。
	if (!CanEnter(Draft.CurrentStep, OutReason))
	{
		return false;
	}
	switch (Draft.CurrentStep)
	{
	case EDBACharacterCreateStep::ZodiacAppearance:
		if (Draft.ZodiacType == EDBAZodiac::None) { OutReason = NSLOCTEXT("DBACharacterCreateDraft", "ZodiacRequired", "请选择生肖。"); return false; }
		return ValidateAppearance(OutReason);
	case EDBACharacterCreateStep::Element:
		if (Draft.ElementType != EDBAElement::None) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "ElementRequired", "请选择元素。"); return false;
	case EDBACharacterCreateStep::FiveCamp:
		if (Draft.FiveCampType != EDBAFiveCamp::None) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "FiveCampRequired", "请选择五营。"); return false;
	case EDBACharacterCreateStep::ConfirmName:
		if (!Draft.CharacterName.IsEmpty()) return true;
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "NameRequired", "请输入角色名称。"); return false;
	default: OutReason = NSLOCTEXT("DBACharacterCreateDraft", "UnknownStep", "角色创建步骤无效。"); return false;
	}
}

bool UDBACharacterCreateDraftSubsystem::Validate(FText& OutReason) const
{
	return Draft.CurrentStep == EDBACharacterCreateStep::ConfirmName && CanLeave(OutReason);
}

bool UDBACharacterCreateDraftSubsystem::Next(FText& OutReason)
{
	// ConfirmName 是终点；提交请求由 Flow 单独发起，避免 Next 隐式产生网络副作用。
	if (!CanLeave(OutReason))
	{
		return false;
	}
	if (Draft.CurrentStep == EDBACharacterCreateStep::ConfirmName)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "AlreadyConfirm", "当前已处于创建确认步骤。");
		return false;
	}
	Draft.CurrentStep = static_cast<EDBACharacterCreateStep>(static_cast<uint8>(Draft.CurrentStep) + 1);
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::Back(FText& OutReason)
{
	OutReason = FText::GetEmpty();
	if (Draft.CurrentStep == EDBACharacterCreateStep::ZodiacAppearance)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "AtFirstStep", "当前已处于角色创建的第一步。");
		return false;
	}
	Draft.CurrentStep = static_cast<EDBACharacterCreateStep>(static_cast<uint8>(Draft.CurrentStep) - 1);
	BroadcastDraftChanged();
	return true;
}

bool UDBACharacterCreateDraftSubsystem::BuildCreateRequest(FDBACharacterCreateRequest& OutRequest, FText& OutReason) const
{
	// 仅在四步全部完成后才允许投影为 DTO；Appearance 由调用方以领域值单独传给 Roster。
	if (!Validate(OutReason))
	{
		return false;
	}
	OutRequest = FDBACharacterCreateRequest();
	OutRequest.CharacterName = Draft.CharacterName;
	OutRequest.Zodiac = Draft.ZodiacType;
	OutRequest.DefaultZodiac = Draft.ZodiacType;
	OutRequest.PrimaryElement = Draft.ElementType;
	OutRequest.DefaultElement = Draft.ElementType;
	OutRequest.FiveCamp = Draft.FiveCampType;
	OutRequest.DefaultFiveCamp = Draft.FiveCampType;
	return true;
}

bool UDBACharacterCreateDraftSubsystem::SerializeRecovery(FString& OutJson) const
{
	// 这是本地临时恢复格式，不包含 AccountId、Token、Password 或任何服务端角色标识。
	FText Reason;
	if (Draft.ZodiacType == EDBAZodiac::None)
	{
		OutJson.Reset();
		return false;
	}
	FString AppearanceJson;
	if (!DBACharacterAppearanceSerialization::ToJson(Draft.Appearance, AppearanceJson)) return false;
	const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetNumberField(TEXT("step"), static_cast<uint8>(Draft.CurrentStep));
	Object->SetStringField(TEXT("zodiac"), StaticEnum<EDBAZodiac>()->GetNameStringByValue(static_cast<int64>(Draft.ZodiacType)));
	Object->SetStringField(TEXT("element"), StaticEnum<EDBAElement>()->GetNameStringByValue(static_cast<int64>(Draft.ElementType)));
	Object->SetStringField(TEXT("fiveCamp"), StaticEnum<EDBAFiveCamp>()->GetNameStringByValue(static_cast<int64>(Draft.FiveCampType)));
	Object->SetStringField(TEXT("name"), Draft.CharacterName);
	Object->SetStringField(TEXT("appearance"), AppearanceJson);
	Object->SetStringField(TEXT("fixedSkillBuildRowId"), Draft.FixedSkillBuildRowId.ToString());
	Object->SetStringField(TEXT("previewSummary"), Draft.PreviewSummary.ToString());
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	return FJsonSerializer::Serialize(Object, Writer);
}

bool UDBACharacterCreateDraftSubsystem::RestoreRecovery(const FString& Json, FText& OutReason)
{
	// 恢复只重建内存草稿，不调用 API，不会把未创建角色伪装成权威角色。
	TSharedPtr<FJsonObject> Object;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object) || !Object.IsValid())
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "RecoveryFormatInvalid", "角色创建临时恢复数据格式无效。"); return false;
	}
	FString ZodiacName, ElementName, CampName, AppearanceJson;
	if (!Object->TryGetStringField(TEXT("zodiac"), ZodiacName) || !Object->TryGetStringField(TEXT("element"), ElementName)
		|| !Object->TryGetStringField(TEXT("fiveCamp"), CampName) || !Object->TryGetStringField(TEXT("appearance"), AppearanceJson))
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "RecoveryIncomplete", "角色创建临时恢复数据不完整。"); return false;
	}
	int64 ZodiacValue, ElementValue, CampValue;
	if (!ParseEnumValue(StaticEnum<EDBAZodiac>(), ZodiacName, ZodiacValue) || !ParseEnumValue(StaticEnum<EDBAElement>(), ElementName, ElementValue)
		|| !ParseEnumValue(StaticEnum<EDBAFiveCamp>(), CampName, CampValue) || ZodiacValue == 0)
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "RecoveryChoiceInvalid", "角色创建临时恢复数据包含无效选择。"); return false;
	}
	FDBACharacterAppearance RestoredAppearance;
	if (!DBACharacterAppearanceSerialization::FromJson(AppearanceJson, RestoredAppearance))
	{
		OutReason = NSLOCTEXT("DBACharacterCreateDraft", "RecoveryAppearanceInvalid", "角色创建临时恢复数据包含无效外观。"); return false;
	}
	BeginDraft();
	Draft.ZodiacType = static_cast<EDBAZodiac>(ZodiacValue);
	Draft.ElementType = static_cast<EDBAElement>(ElementValue);
	Draft.FiveCampType = static_cast<EDBAFiveCamp>(CampValue);
	Draft.Appearance = RestoredAppearance;
	Object->TryGetStringField(TEXT("name"), Draft.CharacterName);
	FString FixedSkillBuildRowId, PreviewSummary;
	Object->TryGetStringField(TEXT("fixedSkillBuildRowId"), FixedSkillBuildRowId);
	Object->TryGetStringField(TEXT("previewSummary"), PreviewSummary);
	Draft.FixedSkillBuildRowId = FName(*FixedSkillBuildRowId);
	Draft.PreviewSummary = FText::FromString(PreviewSummary);
	double Step = 0.0; Object->TryGetNumberField(TEXT("step"), Step);
	Draft.CurrentStep = static_cast<EDBACharacterCreateStep>(FMath::Clamp(static_cast<int32>(Step), 0, static_cast<int32>(EDBACharacterCreateStep::ConfirmName)));
	++DraftRequestVersion;
	RequestZodiacDefaultsAsync(Draft.ZodiacType, DraftRequestVersion);
	if (!ValidateAppearance(OutReason)) return false;
	BroadcastDraftChanged();
	return true;
}

void UDBACharacterCreateDraftSubsystem::BroadcastDraftChanged()
{
	DraftChanged.Broadcast(Draft);
}

void UDBACharacterCreateDraftSubsystem::RequestAppearanceCatalogAsync()
{
	// 外观目录由 DeveloperSettings 软引用配置；空配置保持默认外观可用，但关闭随机外观入口。
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (!Settings || Settings->CharacterAppearanceCatalog.IsNull()) return;
	TWeakObjectPtr<UDBACharacterCreateDraftSubsystem> WeakThis(this);
	AppearanceCatalogLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Settings->CharacterAppearanceCatalog.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda([WeakThis, CatalogPath = Settings->CharacterAppearanceCatalog.ToSoftObjectPath()]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->AppearanceCatalog = Cast<UDBAAppearanceCatalogDataAsset>(CatalogPath.ResolveObject());
				WeakThis->AppearanceCatalogLoadHandle.Reset();
				WeakThis->BroadcastDraftChanged();
			}
		}));
}

void UDBACharacterCreateDraftSubsystem::RequestZodiacDefaultsAsync(const EDBAZodiac Zodiac, const uint32 RequestVersion)
{
	// 使用版本号屏蔽快速切换生肖后的旧异步回调，避免 Rat 的默认外观覆盖 Dragon。
	UDBAZodiacRegistrySubsystem* Registry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAZodiacRegistrySubsystem>() : nullptr;
	if (!Registry) return;
	TWeakObjectPtr<UDBACharacterCreateDraftSubsystem> WeakThis(this);
	Registry->LoadAsync(Zodiac, FDBAOnZodiacHeroAssetLoaded::CreateLambda([WeakThis, RequestVersion](const EDBAZodiac, UDBAZodiacHeroDataAsset* Data)
	{
		if (WeakThis.IsValid() && Data)
		{
			WeakThis->ApplyZodiacDefaults(*Data, RequestVersion);
		}
	}));
}

void UDBACharacterCreateDraftSubsystem::ApplyZodiacDefaults(const UDBAZodiacHeroDataAsset& ZodiacData, const uint32 RequestVersion)
{
	if (RequestVersion != DraftRequestVersion || ZodiacData.ZodiacType != Draft.ZodiacType) return;
	AllowedAppearanceOptionIds.Reset();
	DefaultAppearanceOptionIds = ZodiacData.DefaultAppearanceOptionIds;
	for (const FName OptionId : ZodiacData.AllowedAppearanceOptionIds)
	{
		AllowedAppearanceOptionIds.Add(OptionId);
	}
	// 本地恢复的数据优先于默认值；只有空外观草稿才应用 DataAsset 默认配置。
	TArray<FName> ExistingOptionIds;
	Draft.Appearance.GetSelectedOptionIds(ExistingOptionIds);
	if (ExistingOptionIds.IsEmpty())
	{
		for (const TPair<FName, FName>& Pair : ZodiacData.DefaultAppearanceOptionIds)
		{
			ApplyDefaultOption(Draft.Appearance, Pair.Key, Pair.Value);
		}
	}
	FText NormalizeReason;
	NormalizeAppearance(NormalizeReason);
	BroadcastDraftChanged();
}

bool UDBACharacterCreateDraftSubsystem::ValidateAppearance(FText& OutReason) const
{
	// 本地校验用于尽早反馈；CharacterService 的版本化规则仍是外观合法性的最终权威。
	OutReason = FText::GetEmpty();
	TArray<FName> OptionIds;
	Draft.Appearance.GetSelectedOptionIds(OptionIds);
	for (const FName OptionId : OptionIds)
	{
		if (!IsAppearanceOptionAllowed(OptionId))
		{
			OutReason = FText::Format(NSLOCTEXT("DBACharacterCreateDraft", "AppearanceOptionInvalid", "外观选项 {0} 不适用于当前生肖。"), FText::FromName(OptionId));
			return false;
		}
	}
	return true;
}

bool UDBACharacterCreateDraftSubsystem::IsAppearanceOptionAllowed(const FName OptionId) const
{
	if (OptionId.IsNone()) return true;
	if (!AllowedAppearanceOptionIds.IsEmpty() && !AllowedAppearanceOptionIds.Contains(OptionId)) return false;
	if (const UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get())
	{
		const FDBAAppearanceOptionDefinition* Definition = Catalog->FindOption(OptionId);
		return Definition && Catalog->IsOptionAllowed(*Definition, Draft.ZodiacType);
	}
	return true;
}

bool UDBACharacterCreateDraftSubsystem::RandomizeAppearanceSlot(const EDBAAppearanceSlot Slot, FDBACharacterAppearance& InOutAppearance) const
{
	const UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get();
	if (!Catalog) return false;
	TArray<FName> Options;
	Catalog->GetAvailableOptionIds(Draft.ZodiacType, Slot, Options);
	// 目录的生肖限制与生肖 DataAsset 的额外白名单同时生效，两个来源都不能绕过。
	Options.RemoveAll([this](const FName OptionId) { return !AllowedAppearanceOptionIds.IsEmpty() && !AllowedAppearanceOptionIds.Contains(OptionId); });
	if (Options.IsEmpty()) return false;
	SetAppearanceSlotValue(InOutAppearance, Slot, Options[FMath::RandRange(0, Options.Num() - 1)]);
	return true;
}
