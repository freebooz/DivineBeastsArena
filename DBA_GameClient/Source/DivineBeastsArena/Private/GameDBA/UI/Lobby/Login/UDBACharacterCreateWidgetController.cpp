// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/UI/DBAGameUIManager.h"

UDBACharacterCreateWidgetController::UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterCreateWidgetController::SetCharacterName(const FString& InName)
{
	PendingRequest.CharacterName = InName;
}

void UDBACharacterCreateWidgetController::SetZodiac(EDBAZodiac InZodiac)
{
	PendingRequest.Zodiac = InZodiac;
	PendingRequest.DefaultZodiac = InZodiac;
}

void UDBACharacterCreateWidgetController::SetElement(EDBAElement InElement)
{
	PendingRequest.PrimaryElement = InElement;
	PendingRequest.DefaultElement = InElement;
}

void UDBACharacterCreateWidgetController::SetFiveCamp(EDBAFiveCamp InFiveCamp)
{
	PendingRequest.FiveCamp = InFiveCamp;
	PendingRequest.DefaultFiveCamp = InFiveCamp;
}

void UDBACharacterCreateWidgetController::Submit()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowLobbyLoadingScreen();
			}
		}
		Flow->SubmitCharacterCreation(PendingRequest);
	}
}

UDBALoginFlowSubsystem* UDBACharacterCreateWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}
