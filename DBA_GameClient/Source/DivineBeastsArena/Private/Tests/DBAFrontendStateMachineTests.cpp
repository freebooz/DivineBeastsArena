// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Flow/DBAFrontendStateMachine.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFrontendStateMachineTransitionTest,
	"DBA.Frontend.StateMachine.TransitionGuards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFrontendStateMachineTransitionTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("引导初始化可以进入启动准备"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Bootstrapping, EDBAFrontendState::Startup));
	TestTrue(TEXT("启动准备可以进入登录"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Startup, EDBAFrontendState::Login));
	TestTrue(TEXT("登录成功必须先进入选服"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Login, EDBAFrontendState::ServerSelect));
	TestTrue(TEXT("自动登录成功必须先进入选服"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::AutoLogin, EDBAFrontendState::ServerSelect));
	TestTrue(TEXT("注册成功必须先进入选服"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Register, EDBAFrontendState::ServerSelect));
	TestTrue(TEXT("选服后才能加载角色列表"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::ServerSelect, EDBAFrontendState::CharacterRosterLoading));
	TestTrue(TEXT("登录可以加载角色列表"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Login, EDBAFrontendState::CharacterRosterLoading));
	TestTrue(TEXT("角色创建步骤必须按生肖到元素推进"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::CharacterCreate_Zodiac, EDBAFrontendState::CharacterCreate_Element));
	TestTrue(TEXT("角色创建可取消回角色选择"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::CharacterCreate_FiveCamp, EDBAFrontendState::CharacterSelect));
	TestTrue(TEXT("自动登录失败可回登录"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::AutoLogin, EDBAFrontendState::Login));
	TestTrue(TEXT("服务不可用可从可恢复错误回角色选择"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::RecoverableError, EDBAFrontendState::CharacterSelect));
	TestFalse(TEXT("引导初始化不能直接进入角色选择"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Bootstrapping, EDBAFrontendState::CharacterSelect));
	TestFalse(TEXT("登录不能跳过创建步骤直接确认角色"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::Login, EDBAFrontendState::CharacterCreate_Confirm));
	TestFalse(TEXT("元素步骤不能直接进入进服"), DBAFrontendStateMachine::CanTransition(EDBAFrontendState::CharacterCreate_Element, EDBAFrontendState::EnteringWorld));
	return true;
}

#endif
