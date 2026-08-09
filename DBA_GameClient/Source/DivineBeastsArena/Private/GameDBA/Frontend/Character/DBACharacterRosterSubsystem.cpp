// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"

#include "Dom/JsonObject.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameDBA/Frontend/Account/DBAOnlineAccountJson.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString WriteJsonObject(const TSharedRef<FJsonObject>& Object)
	{
		FString Output;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
		FJsonSerializer::Serialize(Object, Writer);
		return Output;
	}

	bool ReadJsonObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
	{
		return FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), OutObject) && OutObject.IsValid();
	}

	int32 ReadInt(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int32 DefaultValue = 0)
	{
		double Value = DefaultValue;
		return Object.IsValid() && Object->TryGetNumberField(FieldName, Value) ? static_cast<int32>(Value) : DefaultValue;
	}

	void SetOptionIfPresent(const TSharedPtr<FJsonObject>& Options, const TCHAR* FieldName, FName& OutValue)
	{
		FString Value;
		if (Options.IsValid() && Options->TryGetStringField(FieldName, Value))
		{
			OutValue = FName(*Value);
		}
	}

	struct FDBACharacterRosterDtoMapper
	{
		static bool ToDetails(const TSharedPtr<FJsonObject>& Object, FDBACharacterDetails& OutDetails)
		{
			if (!Object.IsValid())
			{
				return false;
			}

			FString CharacterId;
			FString Name;
			if (!Object->TryGetStringField(TEXT("characterId"), CharacterId)
				|| !Object->TryGetStringField(TEXT("name"), Name))
			{
				return false;
			}

			OutDetails = FDBACharacterDetails();
			OutDetails.Summary.CharacterId = FDBACharacterId(CharacterId);
			OutDetails.Summary.CharacterName = Name;
			FString Value;
			Object->TryGetStringField(TEXT("zodiacType"), Value);
			OutDetails.Summary.Zodiac = FDBAOnlineAccountJson::ParseZodiac(Value);
			OutDetails.Summary.DefaultZodiac = OutDetails.Summary.Zodiac;
			Object->TryGetStringField(TEXT("elementType"), Value);
			OutDetails.Summary.PrimaryElement = FDBAOnlineAccountJson::ParseElement(Value);
			OutDetails.Summary.DefaultElement = OutDetails.Summary.PrimaryElement;
			Object->TryGetStringField(TEXT("fiveCampType"), Value);
			OutDetails.Summary.FiveCamp = FDBAOnlineAccountJson::ParseFiveCamp(Value);
			OutDetails.Summary.DefaultFiveCamp = OutDetails.Summary.FiveCamp;
			OutDetails.Summary.Level = ReadInt(Object, TEXT("level"), 1);
			Object->TryGetStringField(TEXT("serverId"), OutDetails.ServerId);
			Object->TryGetBoolField(TEXT("isSelected"), OutDetails.bIsSelected);

			const TSharedPtr<FJsonObject>* AppearanceObject = nullptr;
			if (Object->TryGetObjectField(TEXT("appearance"), AppearanceObject) && AppearanceObject && AppearanceObject->IsValid())
			{
				const TSharedPtr<FJsonObject>* Options = nullptr;
				if ((*AppearanceObject)->TryGetObjectField(TEXT("optionIds"), Options) && Options && Options->IsValid())
				{
					SetOptionIfPresent(*Options, TEXT("gender"), OutDetails.Appearance.GenderId);
					SetOptionIfPresent(*Options, TEXT("bodyType"), OutDetails.Appearance.BodyTypeId);
					SetOptionIfPresent(*Options, TEXT("face"), OutDetails.Appearance.FaceId);
					SetOptionIfPresent(*Options, TEXT("hair"), OutDetails.Appearance.HairId);
					SetOptionIfPresent(*Options, TEXT("hairColor"), OutDetails.Appearance.HairColorId);
					SetOptionIfPresent(*Options, TEXT("skinColor"), OutDetails.Appearance.SkinColorId);
					SetOptionIfPresent(*Options, TEXT("eyeColor"), OutDetails.Appearance.EyeColorId);
					SetOptionIfPresent(*Options, TEXT("marking"), OutDetails.Appearance.MarkingId);
					SetOptionIfPresent(*Options, TEXT("horn"), OutDetails.Appearance.HornId);
					SetOptionIfPresent(*Options, TEXT("ear"), OutDetails.Appearance.EarId);
					SetOptionIfPresent(*Options, TEXT("tail"), OutDetails.Appearance.TailId);
				}
				FString VisualId;
				if ((*AppearanceObject)->TryGetStringField(TEXT("weaponVisualId"), VisualId)) OutDetails.Appearance.WeaponVisualId = FName(*VisualId);
				if ((*AppearanceObject)->TryGetStringField(TEXT("skinId"), VisualId)) OutDetails.Appearance.SkinId = FName(*VisualId);
			}
			return OutDetails.Summary.IsValid();
		}

		static bool ToDetailsArray(const FString& Json, TArray<FDBACharacterDetails>& OutDetails)
		{
			OutDetails.Reset();
			TArray<TSharedPtr<FJsonValue>> Values;
			if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Values))
			{
				return false;
			}
			for (const TSharedPtr<FJsonValue>& Value : Values)
			{
				FDBACharacterDetails Details;
				if (!Value.IsValid() || !ToDetails(Value->AsObject(), Details))
				{
					return false;
				}
				OutDetails.Add(MoveTemp(Details));
			}
			return true;
		}

		static FString ToCreateRequest(const FString& ServerId, const FDBACharacterCreateRequest& Request, const FDBACharacterAppearance& Appearance)
		{
			const EDBAZodiac Zodiac = Request.Zodiac != EDBAZodiac::None ? Request.Zodiac : Request.DefaultZodiac;
			const EDBAElement Element = Request.PrimaryElement != EDBAElement::None ? Request.PrimaryElement : Request.DefaultElement;
			const EDBAFiveCamp Camp = Request.FiveCamp != EDBAFiveCamp::None ? Request.FiveCamp : Request.DefaultFiveCamp;
			const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
			Root->SetStringField(TEXT("serverId"), ServerId);
			Root->SetStringField(TEXT("name"), Request.CharacterName);
			Root->SetStringField(TEXT("zodiacType"), FDBAOnlineAccountJson::ToString(Zodiac));
			Root->SetStringField(TEXT("elementType"), FDBAOnlineAccountJson::ToString(Element));
			Root->SetStringField(TEXT("fiveCampType"), FDBAOnlineAccountJson::ToString(Camp));

			const TSharedRef<FJsonObject> AppearanceObject = MakeShared<FJsonObject>();
			const TSharedRef<FJsonObject> OptionIds = MakeShared<FJsonObject>();
			auto AddOption = [&OptionIds](const TCHAR* Key, const FName Value)
			{
				if (!Value.IsNone()) OptionIds->SetStringField(Key, Value.ToString());
			};
			AddOption(TEXT("gender"), Appearance.GenderId); AddOption(TEXT("bodyType"), Appearance.BodyTypeId);
			AddOption(TEXT("face"), Appearance.FaceId); AddOption(TEXT("hair"), Appearance.HairId);
			AddOption(TEXT("hairColor"), Appearance.HairColorId); AddOption(TEXT("skinColor"), Appearance.SkinColorId);
			AddOption(TEXT("eyeColor"), Appearance.EyeColorId); AddOption(TEXT("marking"), Appearance.MarkingId);
			AddOption(TEXT("horn"), Appearance.HornId); AddOption(TEXT("ear"), Appearance.EarId); AddOption(TEXT("tail"), Appearance.TailId);
			AppearanceObject->SetObjectField(TEXT("optionIds"), OptionIds);
			TArray<TSharedPtr<FJsonValue>> Equipment;
			for (const FName& VisualId : Appearance.EquipmentVisualIds) if (!VisualId.IsNone()) Equipment.Add(MakeShared<FJsonValueString>(VisualId.ToString()));
			AppearanceObject->SetArrayField(TEXT("equipmentVisualIds"), Equipment);
			if (!Appearance.WeaponVisualId.IsNone()) AppearanceObject->SetStringField(TEXT("weaponVisualId"), Appearance.WeaponVisualId.ToString());
			if (!Appearance.SkinId.IsNone()) AppearanceObject->SetStringField(TEXT("skinId"), Appearance.SkinId.ToString());
			Root->SetObjectField(TEXT("appearance"), AppearanceObject);
			return WriteJsonObject(Root);
		}
	};
}

void UDBACharacterRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UDBAApiClientSubsystem>();
	Super::Initialize(Collection);
}

void UDBACharacterRosterSubsystem::Deinitialize()
{
	if (UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr)
	{
		ApiClient->CancelRequestsFor(this);
	}
	ClearCache();
	Super::Deinitialize();
}

bool UDBACharacterRosterSubsystem::IsSupportedInCurrentEnvironment() const
{
	return !IsRunningDedicatedServer();
}

bool UDBACharacterRosterSubsystem::ResolveActiveCacheScope(FString& OutAccountId, FString& OutServerId) const
{
	OutAccountId.Reset(); OutServerId.Reset();
	const UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
	if (!Flow) return false;
	const FDBAFrontendSessionContext& Session = Flow->GetFrontendSessionContext();
	OutAccountId = Session.AccountId;
	OutServerId = Session.ServerId;
	return !OutAccountId.IsEmpty() && !OutServerId.IsEmpty();
}

void UDBACharacterRosterSubsystem::ResetCacheForScope(const FString& AccountId, const FString& ServerId)
{
	CachedCharacters.Reset(); CachedDetailsById.Reset();
	CachedAccountId = AccountId; CachedServerId = ServerId;
}

bool UDBACharacterRosterSubsystem::IsRequestCurrent(const uint64 InRequestGeneration, const FString& AccountId, const FString& ServerId) const
{
	FString CurrentAccountId, CurrentServerId;
	return ResolveActiveCacheScope(CurrentAccountId, CurrentServerId)
		&& IsCacheScopeCurrent(InRequestGeneration, RequestGeneration, AccountId, CurrentAccountId, ServerId, CurrentServerId);
}

bool UDBACharacterRosterSubsystem::ParseCharacterRosterJson(const FString& Json, TArray<FDBACharacterDetails>& OutDetails)
{
	return FDBACharacterRosterDtoMapper::ToDetailsArray(Json, OutDetails);
}

bool UDBACharacterRosterSubsystem::IsCacheScopeCurrent(
	const uint64 ResponseRequestGeneration,
	const uint64 ActiveRequestGeneration,
	const FString& ResponseAccountId,
	const FString& ActiveAccountId,
	const FString& ResponseServerId,
	const FString& ActiveServerId)
{
	return ResponseRequestGeneration == ActiveRequestGeneration
		&& ResponseAccountId == ActiveAccountId
		&& ResponseServerId == ActiveServerId;
}

void UDBACharacterRosterSubsystem::PublishCache()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
	{
		Flow->ApplyCharacterRosterSnapshot(CachedCharacters);
	}
	CharacterRosterChanged.Broadcast(CachedCharacters);
}

void UDBACharacterRosterSubsystem::CompleteValidationFailure(const FString& Message, FDBACharacterRosterCompletion Completion) const
{
	if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, Message));
}

void UDBACharacterRosterSubsystem::ReconcileSelectionAfterRefresh()
{
	UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
	if (!Flow)
	{
		return;
	}

	// 刷新优先保留前台已经选中的稳定 CharacterId；它不存在时才使用服务端选择标记，最后回退列表首项。
	const FString PreviousSelectedId = Flow->GetFrontendSessionContext().SelectedCharacterId;
	FDBACharacterDetails* Selected = PreviousSelectedId.IsEmpty() ? nullptr : CachedDetailsById.Find(PreviousSelectedId);
	if (!Selected)
	{
		for (TPair<FString, FDBACharacterDetails>& Pair : CachedDetailsById)
		{
			if (Pair.Value.bIsSelected)
			{
				Selected = &Pair.Value;
				break;
			}
		}
	}
	if (!Selected && !CachedCharacters.IsEmpty())
	{
		Selected = CachedDetailsById.Find(CachedCharacters[0].CharacterId.ToString());
	}

	for (TPair<FString, FDBACharacterDetails>& Pair : CachedDetailsById)
	{
		Pair.Value.bIsSelected = Selected && Pair.Key == Selected->Summary.CharacterId.ToString();
	}
	if (Selected)
	{
		Selected->bIsSelected = true;
		ApplySelectedCharacter(*Selected);
	}
	else
	{
		// 空列表或原角色已被其他设备删除时不保留空悬 SelectedCharacterId。
		ClearSelectedCharacter();
	}
}

void UDBACharacterRosterSubsystem::RefreshCharacterList(const FString& InServerId, FDBACharacterRosterCompletion Completion)
{
	FString AccountId, SessionServerId;
	if (!ResolveActiveCacheScope(AccountId, SessionServerId) || SessionServerId != InServerId.TrimStartAndEnd())
	{
		CompleteValidationFailure(TEXT("角色列表请求缺少有效的账号或区服上下文。"), MoveTemp(Completion)); return;
	}
	if (CachedAccountId != AccountId || CachedServerId != SessionServerId) ResetCacheForScope(AccountId, SessionServerId);
	if (ActiveRosterRequestId.IsValid())
	{
		if (UDBAApiClientSubsystem* Api = GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>()) Api->CancelRequest(ActiveRosterRequestId);
	}
	const uint64 CurrentRequestGeneration = ++RequestGeneration;
	UDBAApiClientSubsystem* Api = GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>();
	if (!Api) { CompleteValidationFailure(TEXT("角色列表网络服务不可用。"), MoveTemp(Completion)); return; }
	FDBAApiRequest Request; Request.Verb = EDBAApiHttpVerb::Get; Request.Path = FString::Printf(TEXT("/api/v1/characters?serverId=%s"), *SessionServerId);
	ActiveRosterRequestId = Api->Send(Request, this, [this, CurrentRequestGeneration, AccountId, SessionServerId, Completion = MoveTemp(Completion)](const FDBAApiResponse& Response) mutable
	{
		if (!IsRequestCurrent(CurrentRequestGeneration, AccountId, SessionServerId)) return;
		ActiveRosterRequestId.Invalidate();
		if (!Response.Result.bSuccess) { if (Completion) Completion(Response.Result); return; }
		TArray<FDBACharacterDetails> Details;
		if (!FDBACharacterRosterDtoMapper::ToDetailsArray(Response.DomainJson, Details))
		{
			if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("角色列表响应格式无效。"))); return;
		}
		ResetCacheForScope(AccountId, SessionServerId);
		for (const FDBACharacterDetails& Character : Details) { CachedCharacters.Add(Character.Summary); CachedDetailsById.Add(Character.Summary.CharacterId.ToString(), Character); }
		ReconcileSelectionAfterRefresh();
		PublishCache();
		UE_LOG(LogDBACharacter, Log, TEXT("角色列表已刷新：账号=%s，区服=%s，数量=%d。"), *AccountId, *SessionServerId, CachedCharacters.Num());
		if (Completion) Completion(FDBAOperationResult::Success());
	});
}

const FDBACharacterDetails* UDBACharacterRosterSubsystem::FindCachedCharacter(const FDBACharacterId& CharacterId) const
{
	return CharacterId.IsValid() ? CachedDetailsById.Find(CharacterId.ToString()) : nullptr;
}

void UDBACharacterRosterSubsystem::CreateCharacter(const FDBACharacterCreateRequest& RequestData, const FDBACharacterAppearance& Appearance, const FString& IdempotencyKey, FDBACharacterDetailsCompletion Completion)
{
	FString AccountId, ServerId;
	if (!ResolveActiveCacheScope(AccountId, ServerId)) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, TEXT("创建角色前必须先登录并选择区服。")), {}); return; }
	UDBAApiClientSubsystem* Api = GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>();
	if (!Api) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("角色服务不可用。")), {}); return; }
	if (IdempotencyKey.TrimStartAndEnd().IsEmpty())
	{
		if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("角色创建请求缺少幂等键。")), {});
		return;
	}
	if (ActiveCreateRequestId.IsValid())
	{
		if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::DuplicateRequest, TEXT("角色创建请求正在处理中。")), {});
		return;
	}
	FDBAApiRequest ApiRequest; ApiRequest.Verb = EDBAApiHttpVerb::Post; ApiRequest.Path = TEXT("/api/v1/characters"); ApiRequest.JsonBody = FDBACharacterRosterDtoMapper::ToCreateRequest(ServerId, RequestData, Appearance);
	ApiRequest.Headers.Add(TEXT("Idempotency-Key"), IdempotencyKey);
	const uint64 CurrentRequestGeneration = RequestGeneration;
	ActiveCreateRequestId = Api->Send(ApiRequest, this, [this, CurrentRequestGeneration, AccountId, ServerId, Completion = MoveTemp(Completion)](const FDBAApiResponse& Response) mutable
	{
		ActiveCreateRequestId.Invalidate();
		if (!IsRequestCurrent(CurrentRequestGeneration, AccountId, ServerId)) return;
		FDBACharacterDetails Details;
		TSharedPtr<FJsonObject> Object;
		if (!Response.Result.bSuccess) { if (Completion) Completion(Response.Result, {}); return; }
		if (!ReadJsonObject(Response.DomainJson, Object) || !FDBACharacterRosterDtoMapper::ToDetails(Object, Details)) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("创建角色响应格式无效。")), {}); return; }
		CachedDetailsById.Add(Details.Summary.CharacterId.ToString(), Details); CachedCharacters.RemoveAll([&Details](const FDBACharacterSummary& Item) { return Item.CharacterId == Details.Summary.CharacterId; }); CachedCharacters.Add(Details.Summary); PublishCache();
		RefreshCharacterList(ServerId);
		if (Completion) Completion(FDBAOperationResult::Success(), Details);
	});
}

void UDBACharacterRosterSubsystem::CancelCreateCharacterRequest()
{
	if (ActiveCreateRequestId.IsValid())
	{
		if (UDBAApiClientSubsystem* Api = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr)
		{
			Api->CancelRequest(ActiveCreateRequestId);
		}
		ActiveCreateRequestId.Invalidate();
	}
}

void UDBACharacterRosterSubsystem::SelectCreatedCharacterForFrontend(const FDBACharacterDetails& Character)
{
	if (!Character.Summary.CharacterId.IsValid())
	{
		return;
	}
	for (TPair<FString, FDBACharacterDetails>& Pair : CachedDetailsById)
	{
		Pair.Value.bIsSelected = Pair.Key == Character.Summary.CharacterId.ToString();
	}
	FDBACharacterDetails Selected = Character;
	Selected.bIsSelected = true;
	CachedDetailsById.Add(Selected.Summary.CharacterId.ToString(), Selected);
	ApplySelectedCharacter(Selected);
	PublishCache();
}

void UDBACharacterRosterSubsystem::DeleteCharacter(const FDBACharacterId& CharacterId, FDBACharacterRosterCompletion Completion)
{
	FString AccountId, ServerId;
	if (!CharacterId.IsValid() || !ResolveActiveCacheScope(AccountId, ServerId)) { CompleteValidationFailure(TEXT("删除角色请求缺少有效上下文。"), MoveTemp(Completion)); return; }
	UDBAApiClientSubsystem* Api = GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>();
	if (!Api) { CompleteValidationFailure(TEXT("角色服务不可用。"), MoveTemp(Completion)); return; }
	FDBAApiRequest Request; Request.Verb = EDBAApiHttpVerb::Delete; Request.Path = FString::Printf(TEXT("/api/v1/characters/%s"), *CharacterId.ToString()); Request.Headers.Add(TEXT("X-Character-Delete-Confirm"), TEXT("true"));
	const uint64 CurrentRequestGeneration = RequestGeneration;
	Api->Send(Request, this, [this, CurrentRequestGeneration, AccountId, ServerId, CharacterId, Completion = MoveTemp(Completion)](const FDBAApiResponse& Response) mutable
	{
		if (!IsRequestCurrent(CurrentRequestGeneration, AccountId, ServerId)) return;
		if (!Response.Result.bSuccess) { if (Completion) Completion(Response.Result); return; }
		CachedDetailsById.Remove(CharacterId.ToString()); CachedCharacters.RemoveAll([&CharacterId](const FDBACharacterSummary& Item) { return Item.CharacterId == CharacterId; });
		if (UDBAFrontendFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>()) if (Flow->GetFrontendSessionContext().SelectedCharacterId == CharacterId.ToString()) ClearSelectedCharacter();
		PublishCache(); RefreshCharacterList(ServerId); if (Completion) Completion(FDBAOperationResult::Success());
	});
}

void UDBACharacterRosterSubsystem::ApplySelectedCharacter(const FDBACharacterDetails& Character)
{
	if (UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr) Flow->SetSelectedCharacterFromRoster(Character.Summary);
}

void UDBACharacterRosterSubsystem::SelectCharacter(const FDBACharacterId& CharacterId, FDBACharacterDetailsCompletion Completion)
{
	FString AccountId, ServerId;
	const FDBACharacterDetails* Cached = FindCachedCharacter(CharacterId);
	if (!Cached || !ResolveActiveCacheScope(AccountId, ServerId)) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("请选择当前区服中的有效角色。")), {}); return; }
	UDBAApiClientSubsystem* Api = GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>();
	if (!Api) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("角色服务不可用。")), {}); return; }
	FDBAApiRequest Request; Request.Verb = EDBAApiHttpVerb::Post; Request.Path = FString::Printf(TEXT("/api/v1/characters/%s/select"), *CharacterId.ToString()); Request.JsonBody = TEXT("{}");
	const uint64 CurrentRequestGeneration = RequestGeneration;
	Api->Send(Request, this, [this, CurrentRequestGeneration, AccountId, ServerId, CharacterId, Completion = MoveTemp(Completion)](const FDBAApiResponse& Response) mutable
	{
		if (!IsRequestCurrent(CurrentRequestGeneration, AccountId, ServerId)) return;
		if (!Response.Result.bSuccess) { if (Completion) Completion(Response.Result, {}); return; }
		TSharedPtr<FJsonObject> Object; FDBACharacterDetails Details;
		if (!ReadJsonObject(Response.DomainJson, Object) || !FDBACharacterRosterDtoMapper::ToDetails(Object, Details)) { if (Completion) Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("选择角色响应格式无效。")), {}); return; }
		for (TPair<FString, FDBACharacterDetails>& Pair : CachedDetailsById) Pair.Value.bIsSelected = Pair.Key == CharacterId.ToString();
		Details.bIsSelected = true; CachedDetailsById.Add(CharacterId.ToString(), Details); ApplySelectedCharacter(Details); PublishCache();
		if (Completion) Completion(FDBAOperationResult::Success(), Details);
	});
}

void UDBACharacterRosterSubsystem::ClearSelectedCharacter()
{
	for (TPair<FString, FDBACharacterDetails>& Pair : CachedDetailsById) Pair.Value.bIsSelected = false;
	if (UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr) Flow->ClearSelectedCharacterFromRoster();
}

void UDBACharacterRosterSubsystem::ClearCache()
{
	++RequestGeneration;
	CancelCreateCharacterRequest();
	if (ActiveRosterRequestId.IsValid()) if (UDBAApiClientSubsystem* Api = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr) Api->CancelRequest(ActiveRosterRequestId);
	ActiveRosterRequestId.Invalidate(); ResetCacheForScope(FString(), FString()); ClearSelectedCharacter(); PublishCache();
}
