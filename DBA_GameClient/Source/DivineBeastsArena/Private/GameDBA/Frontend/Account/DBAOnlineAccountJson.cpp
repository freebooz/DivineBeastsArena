// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Account/DBAOnlineAccountJson.h"

#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
bool ParseObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject, FString& OutError)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
	{
		OutError = TEXT("JSON 响应格式错误");
		return false;
	}

	return true;
}

FString WriteObject(const TSharedRef<FJsonObject>& Object)
{
	FString Output;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Object, Writer);
	return Output;
}

FString GetStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
{
	FString Value;
	if (Object.IsValid())
	{
		Object->TryGetStringField(FieldName, Value);
	}
	return Value;
}

bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, bool DefaultValue = false)
{
	bool Value = DefaultValue;
	if (Object.IsValid())
	{
		Object->TryGetBoolField(FieldName, Value);
	}
	return Value;
}

int32 GetIntegerField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int32 DefaultValue = 0)
{
	double Value = static_cast<double>(DefaultValue);
	Object->TryGetNumberField(FieldName, Value);
	return static_cast<int32>(Value);
}

int64 GetInteger64Field(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int64 DefaultValue = 0)
{
	double Value = static_cast<double>(DefaultValue);
	Object->TryGetNumberField(FieldName, Value);
	return static_cast<int64>(Value);
}

float GetFloatField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, float DefaultValue)
{
	double Value = static_cast<double>(DefaultValue);
	Object->TryGetNumberField(FieldName, Value);
	return static_cast<float>(Value);
}

TSharedPtr<FJsonObject> ResolvePayloadObject(const TSharedPtr<FJsonObject>& Object)
{
	const TSharedPtr<FJsonObject>* DataObject = nullptr;
	if (Object.IsValid()
		&& Object->TryGetObjectField(TEXT("data"), DataObject)
		&& DataObject
		&& DataObject->IsValid())
	{
		return *DataObject;
	}

	return Object;
}

void ReadCoreAttributes(const TSharedPtr<FJsonObject>& Object, FDBACharacterCoreAttributes& OutAttributes)
{
	const TSharedPtr<FJsonObject>* AttributesObject = nullptr;
	if (!Object->TryGetObjectField(TEXT("coreAttributes"), AttributesObject) || !AttributesObject || !AttributesObject->IsValid())
	{
		return;
	}

	OutAttributes.MaxHealth = GetFloatField(*AttributesObject, TEXT("maxHealth"), OutAttributes.MaxHealth);
	OutAttributes.AttackPower = GetFloatField(*AttributesObject, TEXT("attackPower"), OutAttributes.AttackPower);
	OutAttributes.Defense = GetFloatField(*AttributesObject, TEXT("defense"), OutAttributes.Defense);
	OutAttributes.MoveSpeed = GetFloatField(*AttributesObject, TEXT("moveSpeed"), OutAttributes.MoveSpeed);
	OutAttributes.MaxEnergy = GetFloatField(*AttributesObject, TEXT("maxEnergy"), OutAttributes.MaxEnergy);
	OutAttributes.EnergyRegen = GetFloatField(*AttributesObject, TEXT("energyRegen"), OutAttributes.EnergyRegen);
	OutAttributes.CriticalRate = GetFloatField(*AttributesObject, TEXT("criticalRate"), OutAttributes.CriticalRate);
	OutAttributes.CriticalMultiplier = GetFloatField(*AttributesObject, TEXT("criticalMultiplier"), OutAttributes.CriticalMultiplier);
}

void WriteCoreAttributes(const FDBACharacterCoreAttributes& Attributes, const TSharedRef<FJsonObject>& Object)
{
	TSharedRef<FJsonObject> AttributesObject = MakeShared<FJsonObject>();
	AttributesObject->SetNumberField(TEXT("maxHealth"), Attributes.MaxHealth);
	AttributesObject->SetNumberField(TEXT("attackPower"), Attributes.AttackPower);
	AttributesObject->SetNumberField(TEXT("defense"), Attributes.Defense);
	AttributesObject->SetNumberField(TEXT("moveSpeed"), Attributes.MoveSpeed);
	AttributesObject->SetNumberField(TEXT("maxEnergy"), Attributes.MaxEnergy);
	AttributesObject->SetNumberField(TEXT("energyRegen"), Attributes.EnergyRegen);
	AttributesObject->SetNumberField(TEXT("criticalRate"), Attributes.CriticalRate);
	AttributesObject->SetNumberField(TEXT("criticalMultiplier"), Attributes.CriticalMultiplier);
	Object->SetObjectField(TEXT("coreAttributes"), AttributesObject);
}

FDBACharacterSummary ParseCharacterSummary(const TSharedPtr<FJsonObject>& CharacterObject)
{
	FDBACharacterSummary Summary;
	Summary.CharacterId = FDBACharacterId(GetStringField(CharacterObject, TEXT("characterId")));
	Summary.CharacterName = GetStringField(CharacterObject, TEXT("characterName"));
	Summary.Zodiac = FDBAOnlineAccountJson::ParseZodiac(GetStringField(CharacterObject, TEXT("zodiac")));
	Summary.PrimaryElement = FDBAOnlineAccountJson::ParseElement(GetStringField(CharacterObject, TEXT("primaryElement")));
	Summary.FiveCamp = FDBAOnlineAccountJson::ParseFiveCamp(GetStringField(CharacterObject, TEXT("fiveCamp")));
	Summary.FixedSkillGroupId = FName(*GetStringField(CharacterObject, TEXT("fixedSkillGroupId")));
	Summary.CoreAttributes = FDBACharacterCoreAttributes();
	ReadCoreAttributes(CharacterObject, Summary.CoreAttributes);
	Summary.DefaultZodiac = Summary.Zodiac;
	Summary.DefaultElement = Summary.PrimaryElement;
	Summary.DefaultFiveCamp = Summary.FiveCamp;
	Summary.Level = GetIntegerField(CharacterObject, TEXT("level"), 1);
	Summary.CreateTime = GetInteger64Field(CharacterObject, TEXT("createTime"));
	Summary.LastUsedTime = GetInteger64Field(CharacterObject, TEXT("lastUsedTime"));
	return Summary;
}
}

FString FDBAOnlineAccountJson::BuildLoginRequest(const FDBALoginRequest& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("loginType"), ToString(Request.LoginType));
	Object->SetStringField(TEXT("username"), Request.Email);
	Object->SetStringField(TEXT("email"), Request.Email);
	Object->SetStringField(TEXT("password"), Request.Password);
	Object->SetStringField(TEXT("thirdPartyToken"), Request.ThirdPartyToken);
	Object->SetStringField(TEXT("deviceId"), Request.DeviceId);
	return WriteObject(Object);
}

FString FDBAOnlineAccountJson::BuildGuestLoginRequest(const FString& DeviceId, const FString& DeviceName, const FString& Platform)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("deviceId"), DeviceId);
	Object->SetStringField(TEXT("deviceName"), DeviceName.IsEmpty() ? TEXT("UnrealClient") : DeviceName);
	Object->SetStringField(TEXT("platform"), Platform.IsEmpty() ? TEXT("Windows") : Platform);
	return WriteObject(Object);
}

FString FDBAOnlineAccountJson::BuildCreateCharacterRequest(const FDBACharacterCreateRequest& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	const EDBAZodiac Zodiac = Request.Zodiac != EDBAZodiac::None ? Request.Zodiac : Request.DefaultZodiac;
	const EDBAElement PrimaryElement = Request.PrimaryElement != EDBAElement::None ? Request.PrimaryElement : Request.DefaultElement;
	const EDBAFiveCamp FiveCamp = Request.FiveCamp != EDBAFiveCamp::None ? Request.FiveCamp : Request.DefaultFiveCamp;

	Object->SetStringField(TEXT("characterName"), Request.CharacterName);
	Object->SetStringField(TEXT("zodiac"), ToString(Zodiac));
	Object->SetStringField(TEXT("primaryElement"), ToString(PrimaryElement));
	Object->SetStringField(TEXT("fiveCamp"), ToString(FiveCamp));
	return WriteObject(Object);
}

FString FDBAOnlineAccountJson::BuildSelectCharacterRequest(const FDBACharacterId& CharacterId)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("characterId"), CharacterId.ToString());
	return WriteObject(Object);
}

bool FDBAOnlineAccountJson::ParseLoginResponse(const FString& Json, FDBALoginResponse& OutResponse, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	OutResponse = FDBALoginResponse();
	OutResponse.bSuccess = GetBoolField(Object, TEXT("success"));
	OutResponse.ErrorMessage = GetStringField(Object, TEXT("error"));
	if (OutResponse.ErrorMessage.IsEmpty())
	{
		OutResponse.ErrorMessage = GetStringField(Object, TEXT("message"));
	}

	const TSharedPtr<FJsonObject>* DataObject = nullptr;
	if (Object->TryGetObjectField(TEXT("data"), DataObject) && DataObject && DataObject->IsValid())
	{
		const TSharedPtr<FJsonObject>& Data = *DataObject;
		OutResponse.SessionToken = GetStringField(Data, TEXT("accessToken"));
		if (OutResponse.SessionToken.IsEmpty())
		{
			OutResponse.SessionToken = GetStringField(Data, TEXT("token"));
		}
		OutResponse.RefreshToken = GetStringField(Data, TEXT("refreshToken"));
		OutResponse.PlayerId = GetStringField(Data, TEXT("playerId"));
		OutResponse.AccountInfo.AccountId = FDBAAccountId(OutResponse.PlayerId);
		OutResponse.AccountInfo.DisplayName = GetStringField(Data, TEXT("nickname"));
		if (OutResponse.AccountInfo.DisplayName.IsEmpty())
		{
			OutResponse.AccountInfo.DisplayName = GetStringField(Data, TEXT("displayName"));
		}
		OutResponse.AccountInfo.LoginType = ParseLoginType(GetStringField(Data, TEXT("loginType")));
		OutResponse.AccountInfo.Status = EDBAAccountStatus::Normal;
		OutResponse.AccountInfo.Level = 1;
		return true;
	}

	OutResponse.SessionToken = GetStringField(Object, TEXT("token"));
	OutResponse.RefreshToken = GetStringField(Object, TEXT("refreshToken"));

	const TSharedPtr<FJsonObject>* AccountObject = nullptr;
	if (Object->TryGetObjectField(TEXT("account"), AccountObject) && AccountObject && AccountObject->IsValid())
	{
		OutResponse.AccountInfo.AccountId = FDBAAccountId(GetStringField(*AccountObject, TEXT("accountId")));
		OutResponse.PlayerId = GetStringField(*AccountObject, TEXT("playerId"));
		OutResponse.AccountInfo.DisplayName = GetStringField(*AccountObject, TEXT("displayName"));
		OutResponse.AccountInfo.LoginType = ParseLoginType(GetStringField(*AccountObject, TEXT("loginType")));
		OutResponse.AccountInfo.Status = ParseAccountStatus(GetStringField(*AccountObject, TEXT("status")));
		OutResponse.AccountInfo.Level = GetIntegerField(*AccountObject, TEXT("level"), 1);
		OutResponse.AccountInfo.Experience = GetIntegerField(*AccountObject, TEXT("experience"));
		OutResponse.AccountInfo.CreateTime = GetInteger64Field(*AccountObject, TEXT("createTime"));
		OutResponse.AccountInfo.LastLoginTime = GetInteger64Field(*AccountObject, TEXT("lastLoginTime"));
	}

	return true;
}

bool FDBAOnlineAccountJson::ParseGeneratedPlayerNameResponse(
	const FString& Json,
	FString& OutPlayerName,
	FString& OutError)
{
	TSharedPtr<FJsonObject> Root;
	if (!ParseObject(Json, Root, OutError))
	{
		return false;
	}

	const TSharedPtr<FJsonObject> Payload = ResolvePayloadObject(Root);
	OutPlayerName = GetStringField(Payload, TEXT("nickname")).TrimStartAndEnd();
	if (OutPlayerName.IsEmpty())
	{
		OutError = TEXT("玩家名接口响应缺少 nickname 字段。");
		return false;
	}

	return true;
}

bool FDBAOnlineAccountJson::ParseCharacterListResponse(const FString& Json, TArray<FDBACharacterSummary>& OutCharacters, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	OutCharacters.Reset();

	if (!GetBoolField(Object, TEXT("success")))
	{
		OutError = GetStringField(Object, TEXT("error"));
		if (OutError.IsEmpty())
		{
			OutError = GetStringField(Object, TEXT("message"));
		}
		return true;
	}

	const TSharedPtr<FJsonObject> PayloadObject = ResolvePayloadObject(Object);
	const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
	if ((!PayloadObject.IsValid() || (!PayloadObject->TryGetArrayField(TEXT("characters"), Items) && !PayloadObject->TryGetArrayField(TEXT("items"), Items)))
		&& !Object->TryGetArrayField(TEXT("data"), Items))
	{
		OutError = TEXT("响应缺少 characters 数组");
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Item : *Items)
	{
		const TSharedPtr<FJsonObject> CharacterObject = Item.IsValid() ? Item->AsObject() : nullptr;
		if (CharacterObject.IsValid())
		{
			OutCharacters.Add(ParseCharacterSummary(CharacterObject));
		}
	}

	return true;
}

bool FDBAOnlineAccountJson::ParseCreateCharacterResponse(const FString& Json, FDBACharacterCreateResponse& OutResponse, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	OutResponse = FDBACharacterCreateResponse();
	OutResponse.bSuccess = GetBoolField(Object, TEXT("success"));
	OutResponse.ErrorMessage = GetStringField(Object, TEXT("error"));
	if (OutResponse.ErrorMessage.IsEmpty())
	{
		OutResponse.ErrorMessage = GetStringField(Object, TEXT("message"));
	}

	const TSharedPtr<FJsonObject> PayloadObject = ResolvePayloadObject(Object);
	const TSharedPtr<FJsonObject>* CharacterObject = nullptr;
	if (PayloadObject.IsValid()
		&& PayloadObject->TryGetObjectField(TEXT("character"), CharacterObject)
		&& CharacterObject
		&& CharacterObject->IsValid())
	{
		OutResponse.CharacterSummary = ParseCharacterSummary(*CharacterObject);
	}
	else if (PayloadObject.IsValid() && !GetStringField(PayloadObject, TEXT("characterId")).IsEmpty())
	{
		OutResponse.CharacterSummary = ParseCharacterSummary(PayloadObject);
	}

	return true;
}

bool FDBAOnlineAccountJson::ParseSelectCharacterResponse(const FString& Json, FDBACharacterId& OutCharacterId, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	OutCharacterId = FDBACharacterId();
	if (!GetBoolField(Object, TEXT("success")))
	{
		OutError = GetStringField(Object, TEXT("error"));
		if (OutError.IsEmpty())
		{
			OutError = GetStringField(Object, TEXT("message"));
		}
		return true;
	}

	const TSharedPtr<FJsonObject> PayloadObject = ResolvePayloadObject(Object);
	FString SelectedId = GetStringField(Object, TEXT("selectedCharacterId"));
	if (SelectedId.IsEmpty() && PayloadObject.IsValid())
	{
		SelectedId = GetStringField(PayloadObject, TEXT("selectedCharacterId"));
	}
	if (SelectedId.IsEmpty())
	{
		const TSharedPtr<FJsonObject>* CharacterObject = nullptr;
		if (PayloadObject.IsValid()
			&& PayloadObject->TryGetObjectField(TEXT("character"), CharacterObject)
			&& CharacterObject
			&& CharacterObject->IsValid())
		{
			SelectedId = GetStringField(*CharacterObject, TEXT("characterId"));
		}
	}
	if (SelectedId.IsEmpty() && PayloadObject.IsValid())
	{
		SelectedId = GetStringField(PayloadObject, TEXT("characterId"));
	}

	OutCharacterId = FDBACharacterId(SelectedId);
	return true;
}

EDBALoginType FDBAOnlineAccountJson::ParseLoginType(const FString& Value)
{
	if (Value.Equals(TEXT("Guest"), ESearchCase::IgnoreCase)) return EDBALoginType::Guest;
	if (Value.Equals(TEXT("Email"), ESearchCase::IgnoreCase)) return EDBALoginType::Email;
	if (Value.Equals(TEXT("ThirdParty"), ESearchCase::IgnoreCase)) return EDBALoginType::ThirdParty;
	return EDBALoginType::None;
}

EDBAAccountStatus FDBAOnlineAccountJson::ParseAccountStatus(const FString& Value)
{
	if (Value.Equals(TEXT("Banned"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::Banned;
	if (Value.Equals(TEXT("Frozen"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::Frozen;
	if (Value.Equals(TEXT("PendingVerification"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::PendingVerification;
	return EDBAAccountStatus::Normal;
}

EDBAZodiac FDBAOnlineAccountJson::ParseZodiac(const FString& Value)
{
	if (Value.Equals(TEXT("Rat"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rat;
	if (Value.Equals(TEXT("Ox"), ESearchCase::IgnoreCase)) return EDBAZodiac::Ox;
	if (Value.Equals(TEXT("Tiger"), ESearchCase::IgnoreCase)) return EDBAZodiac::Tiger;
	if (Value.Equals(TEXT("Rabbit"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rabbit;
	if (Value.Equals(TEXT("Dragon"), ESearchCase::IgnoreCase)) return EDBAZodiac::Dragon;
	if (Value.Equals(TEXT("Snake"), ESearchCase::IgnoreCase)) return EDBAZodiac::Snake;
	if (Value.Equals(TEXT("Horse"), ESearchCase::IgnoreCase)) return EDBAZodiac::Horse;
	if (Value.Equals(TEXT("Goat"), ESearchCase::IgnoreCase)) return EDBAZodiac::Goat;
	if (Value.Equals(TEXT("Monkey"), ESearchCase::IgnoreCase)) return EDBAZodiac::Monkey;
	if (Value.Equals(TEXT("Rooster"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rooster;
	if (Value.Equals(TEXT("Dog"), ESearchCase::IgnoreCase)) return EDBAZodiac::Dog;
	if (Value.Equals(TEXT("Pig"), ESearchCase::IgnoreCase)) return EDBAZodiac::Pig;
	return EDBAZodiac::None;
}

EDBAElement FDBAOnlineAccountJson::ParseElement(const FString& Value)
{
	if (Value.Equals(TEXT("Fire"), ESearchCase::IgnoreCase)) return EDBAElement::Fire;
	if (Value.Equals(TEXT("Water"), ESearchCase::IgnoreCase)) return EDBAElement::Water;
	if (Value.Equals(TEXT("Wood"), ESearchCase::IgnoreCase)) return EDBAElement::Wood;
	if (Value.Equals(TEXT("Gold"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Metal"), ESearchCase::IgnoreCase)) return EDBAElement::Gold;
	if (Value.Equals(TEXT("Earth"), ESearchCase::IgnoreCase)) return EDBAElement::Earth;
	return EDBAElement::None;
}

EDBAFiveCamp FDBAOnlineAccountJson::ParseFiveCamp(const FString& Value)
{
	if (Value.Equals(TEXT("East"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("QingLong"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::East;
	if (Value.Equals(TEXT("West"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("BaiHu"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::West;
	if (Value.Equals(TEXT("South"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ZhuQue"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::South;
	if (Value.Equals(TEXT("North"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("XuanWu"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::North;
	if (Value.Equals(TEXT("Center"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("QiLin"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::Center;
	return EDBAFiveCamp::None;
}

FString FDBAOnlineAccountJson::ToString(EDBALoginType Value)
{
	switch (Value)
	{
	case EDBALoginType::Guest: return TEXT("Guest");
	case EDBALoginType::Email: return TEXT("Email");
	case EDBALoginType::ThirdParty: return TEXT("ThirdParty");
	default: return TEXT("None");
	}
}

FString FDBAOnlineAccountJson::ToString(EDBAAccountStatus Value)
{
	switch (Value)
	{
	case EDBAAccountStatus::Banned: return TEXT("Banned");
	case EDBAAccountStatus::Frozen: return TEXT("Frozen");
	case EDBAAccountStatus::PendingVerification: return TEXT("PendingVerification");
	default: return TEXT("Normal");
	}
}

FString FDBAOnlineAccountJson::ToString(EDBAZodiac Value)
{
	switch (Value)
	{
	case EDBAZodiac::Rat: return TEXT("Rat");
	case EDBAZodiac::Ox: return TEXT("Ox");
	case EDBAZodiac::Tiger: return TEXT("Tiger");
	case EDBAZodiac::Rabbit: return TEXT("Rabbit");
	case EDBAZodiac::Dragon: return TEXT("Dragon");
	case EDBAZodiac::Snake: return TEXT("Snake");
	case EDBAZodiac::Horse: return TEXT("Horse");
	case EDBAZodiac::Goat: return TEXT("Goat");
	case EDBAZodiac::Monkey: return TEXT("Monkey");
	case EDBAZodiac::Rooster: return TEXT("Rooster");
	case EDBAZodiac::Dog: return TEXT("Dog");
	case EDBAZodiac::Pig: return TEXT("Pig");
	default: return TEXT("None");
	}
}

FString FDBAOnlineAccountJson::ToString(EDBAElement Value)
{
	switch (Value)
	{
	case EDBAElement::Fire: return TEXT("Fire");
	case EDBAElement::Water: return TEXT("Water");
	case EDBAElement::Wood: return TEXT("Wood");
	case EDBAElement::Gold: return TEXT("Gold");
	case EDBAElement::Earth: return TEXT("Earth");
	default: return TEXT("None");
	}
}

FString FDBAOnlineAccountJson::ToString(EDBAFiveCamp Value)
{
	switch (Value)
	{
	case EDBAFiveCamp::East: return TEXT("East");
	case EDBAFiveCamp::West: return TEXT("West");
	case EDBAFiveCamp::South: return TEXT("South");
	case EDBAFiveCamp::North: return TEXT("North");
	case EDBAFiveCamp::Center: return TEXT("Center");
	default: return TEXT("None");
	}
}
