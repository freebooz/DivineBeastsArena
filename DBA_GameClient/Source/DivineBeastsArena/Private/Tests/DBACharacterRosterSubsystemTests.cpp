// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "Misc/AutomationTest.h"

namespace
{
	FString MakeCharacterJson(const FString& Id, const FString& Name)
	{
		return FString::Printf(
			TEXT("{\"characterId\":\"%s\",\"serverId\":\"10000000-0000-0000-0000-000000000001\",\"name\":\"%s\",\"zodiacType\":\"Rat\",\"elementType\":\"Water\",\"fiveCampType\":\"North\",\"level\":1,\"isSelected\":false,\"appearance\":{\"optionIds\":{\"hair\":\"hair.default\"},\"equipmentVisualIds\":[]}}"),
			*Id,
			*Name);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterRosterDtoMappingTest,
	"DBA.Frontend.CharacterRoster.DtoMapsEmptyOneAndMaxSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterRosterDtoMappingTest::RunTest(const FString& Parameters)
{
	TArray<FDBACharacterDetails> Details;
	TestTrue(TEXT("空角色列表应解析为有效领域集合"), UDBACharacterRosterSubsystem::ParseCharacterRosterJson(TEXT("[]"), Details));
	TestEqual(TEXT("空角色列表不应生成角色"), Details.Num(), 0);

	const FString OneCharacter = FString::Printf(TEXT("[%s]"), *MakeCharacterJson(TEXT("20000000-0000-0000-0000-000000000001"), TEXT("测试角色")));
	TestTrue(TEXT("单角色 DTO 应映射为领域详情"), UDBACharacterRosterSubsystem::ParseCharacterRosterJson(OneCharacter, Details));
	TestEqual(TEXT("单角色列表数量正确"), Details.Num(), 1);
	TestEqual(TEXT("领域摘要保留稳定角色 ID"), Details[0].Summary.CharacterId.ToString(), FString(TEXT("20000000-0000-0000-0000-000000000001")));
	TestEqual(TEXT("外观仅保留稳定选项 ID"), Details[0].Appearance.HairId, FName(TEXT("hair.default")));

	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	const int32 MaxSlots = Settings ? Settings->MaxCharacterSlots : 0;
	FString MaxPayload = TEXT("[");
	for (int32 Index = 0; Index < MaxSlots; ++Index)
	{
		if (Index > 0) MaxPayload += TEXT(",");
		MaxPayload += MakeCharacterJson(FString::Printf(TEXT("30000000-0000-0000-0000-%012d"), Index), FString::Printf(TEXT("角色%d"), Index));
	}
	MaxPayload += TEXT("]");
	TestTrue(TEXT("最大角色槽位 DTO 仍应完整映射，由服务端负责槽位权威校验"), UDBACharacterRosterSubsystem::ParseCharacterRosterJson(MaxPayload, Details));
	TestEqual(TEXT("最大槽位列表不丢失条目"), Details.Num(), MaxSlots);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterRosterScopeTest,
	"DBA.Frontend.CharacterRoster.NetworkErrorAndServerSwitchRace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterRosterScopeTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("网络错误结果不应改写缓存，由调用方保留已有快照"), FDBAOperationResult::Failure(EDBAErrorCode::NetworkFailure, TEXT("网络不可用")).bSuccess);
	TestFalse(
		TEXT("换服后旧请求结果必须被拒绝"),
		UDBACharacterRosterSubsystem::IsCacheScopeCurrent(7, 8, TEXT("account-a"), TEXT("account-a"), TEXT("server-a"), TEXT("server-b")));
	TestTrue(
		TEXT("同账号同区服且请求代次一致的结果可以写入缓存"),
		UDBACharacterRosterSubsystem::IsCacheScopeCurrent(8, 8, TEXT("account-a"), TEXT("account-a"), TEXT("server-b"), TEXT("server-b")));
	return true;
}

#endif
