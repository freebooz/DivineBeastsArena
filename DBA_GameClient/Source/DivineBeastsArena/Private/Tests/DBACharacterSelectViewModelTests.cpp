// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/CharacterSelection/DBACharacterSelectViewModel.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterSelectViewModelTest,
	"DBA.Frontend.CharacterSelect.SelectionAndIndependentLoading",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterSelectViewModelTest::RunTest(const FString& Parameters)
{
	UDBACharacterSelectViewModel* ViewModel = NewObject<UDBACharacterSelectViewModel>();
	FDBACharacterSummary First; First.CharacterId = FDBACharacterId(TEXT("character-a")); First.CharacterName = TEXT("角色甲");
	FDBACharacterSummary Second; Second.CharacterId = FDBACharacterId(TEXT("character-b")); Second.CharacterName = TEXT("角色乙");
	TArray<FDBACharacterSummary> Characters = { First, Second };

	ViewModel->ApplyRoster(Characters, nullptr);
	TestEqual(TEXT("列表首次加载安全选择第一个角色"), ViewModel->GetSelectedCharacterId().ToString(), First.CharacterId.ToString());
	ViewModel->SetRosterLoading(true);
	ViewModel->SetPreviewLoading(false);
	TestTrue(TEXT("列表刷新可单独显示加载状态"), ViewModel->IsRosterLoading());
	TestFalse(TEXT("列表刷新不应伪造预览加载"), ViewModel->IsPreviewLoading());
	ViewModel->SetRosterLoading(false);
	ViewModel->SetPreviewLoading(true);
	TestTrue(TEXT("预览加载可独立显示，不阻塞角色列表状态"), ViewModel->IsPreviewLoading());
	TestTrue(TEXT("预览未完成时禁止进入游戏"), !ViewModel->CanEnterGame());
	return true;
}

#endif
