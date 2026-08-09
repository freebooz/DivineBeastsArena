// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterCreateDraftStateTest,
	"DBA.Frontend.CharacterCreate.DraftFourStepAndRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterCreateDraftStateTest::RunTest(const FString& Parameters)
{
	UDBACharacterCreateDraftSubsystem* Draft = NewObject<UDBACharacterCreateDraftSubsystem>();
	FText Reason;

	TestTrue(TEXT("生肖草稿可进入第一步"), Draft->CanEnter(EDBACharacterCreateStep::ZodiacAppearance, Reason));
	TestFalse(TEXT("未选择生肖时不得离开第一步"), Draft->CanLeave(Reason));
	TestTrue(TEXT("选择生肖写入唯一创建草稿"), Draft->SetZodiac(EDBAZodiac::Rat));
	TestTrue(TEXT("生肖步骤可推进到元素步骤"), Draft->Next(Reason));
	TestEqual(TEXT("第二步必须为元素"), Draft->GetDraft().CurrentStep, EDBACharacterCreateStep::Element);
	TestTrue(TEXT("元素选择可写入草稿"), Draft->SetElement(EDBAElement::Water));
	TestTrue(TEXT("元素步骤可推进到五营步骤"), Draft->Next(Reason));
	TestTrue(TEXT("五营选择可写入草稿"), Draft->SetFiveCamp(EDBAFiveCamp::East));
	TestTrue(TEXT("五营步骤可推进到确认步骤"), Draft->Next(Reason));
	TestFalse(TEXT("未填写名称时不得提交创建"), Draft->Validate(Reason));
	TestTrue(TEXT("名称写入草稿"), Draft->SetCharacterName(TEXT("草稿角色")));
	TestTrue(TEXT("四步完成后可生成创建请求"), Draft->Validate(Reason));

	FDBACharacterCreateRequest Request;
	TestTrue(TEXT("请求必须从草稿生成"), Draft->BuildCreateRequest(Request, Reason));
	TestEqual(TEXT("创建请求保留生肖"), Request.Zodiac, EDBAZodiac::Rat);
	TestEqual(TEXT("创建请求保留元素"), Request.PrimaryElement, EDBAElement::Water);
	TestEqual(TEXT("创建请求保留五营"), Request.FiveCamp, EDBAFiveCamp::East);

	FString RecoveryJson;
	TestTrue(TEXT("草稿可序列化为本地临时恢复数据"), Draft->SerializeRecovery(RecoveryJson));
	Draft->ResetDraft();
	TestFalse(TEXT("重置后不再保留未创建角色名称"), !Draft->GetDraft().CharacterName.IsEmpty());
	TestTrue(TEXT("恢复数据不会创建服务端角色，但可重建本地草稿"), Draft->RestoreRecovery(RecoveryJson, Reason));
	TestEqual(TEXT("恢复后仍停留确认步骤"), Draft->GetDraft().CurrentStep, EDBACharacterCreateStep::ConfirmName);
	return true;
}

#endif
