// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/UDBAMainLobbyWidgetController.h"

/**
 * 构造函数
 * 初始化主大厅 Widget 控制器
 */
UDBAMainLobbyWidgetController::UDBAMainLobbyWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 请求获取队伍信息
 * 获取当前玩家的队伍信息并通过事件通知 UI
 * 目前使用桩数据，实际实现需要调用 PartySubsystem
 */
void UDBAMainLobbyWidgetController::RequestPartyInfo()
{
	FDBAPartyInfo StubPartyInfo;
	StubPartyInfo.PartyId = FDBAPartyId(TEXT("party_001"));
	StubPartyInfo.LeaderAccountId = FDBAAccountId(TEXT("player_001"));
	OnPartyInfoReady.Broadcast(StubPartyInfo);
}

/**
 * 请求切换阵营主题
 * @param FiveCamp 要切换的阵营类型
 * 调用 LobbySubsystem 的 ApplyCampTheme 方法应用主题
 */
void UDBAMainLobbyWidgetController::RequestSwitchFiveCampTheme(uint8 FiveCamp)
{
}

/**
 * 请求导航到新手村
 * 处理进入新手村的请求
 */
void UDBAMainLobbyWidgetController::RequestNavigateToNewbieVillage()
{
}

/**
 * 请求导航到练习模式
 * 处理进入练习模式的请求
 */
void UDBAMainLobbyWidgetController::RequestNavigateToPractice()
{
}

/**
 * 请求退出游戏
 * 处理退出游戏的请求
 */
void UDBAMainLobbyWidgetController::RequestExitGame()
{
}
