// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栦富澶у巺 Widget 鎺у埗鍣? */
UDBAMainLobbyWidgetController::UDBAMainLobbyWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 璇锋眰鑾峰彇闃熶紞淇℃伅
 * 鑾峰彇褰撳墠鐜╁鐨勯槦浼嶄俊鎭苟閫氳繃浜嬩欢閫氱煡 UI
 * 鐩墠浣跨敤妗╂暟鎹紝瀹為檯瀹炵幇闇€瑕佽皟鐢?PartySubsystem
 */
void UDBAMainLobbyWidgetController::RequestPartyInfo()
{
	FDBAPartyInfo StubPartyInfo;
	StubPartyInfo.PartyId = FDBAPartyId(TEXT("party_001"));
	StubPartyInfo.LeaderAccountId = FDBAAccountId(TEXT("player_001"));
	OnPartyInfoReady.Broadcast(StubPartyInfo);
}

/**
 * 璇锋眰鍒囨崲闃佃惀涓婚
 * @param FiveCamp 瑕佸垏鎹㈢殑闃佃惀绫诲瀷
 * 璋冪敤 LobbySubsystem 鐨?ApplyCampTheme 鏂规硶搴旂敤涓婚
 */
void UDBAMainLobbyWidgetController::RequestSwitchFiveCampTheme(uint8 FiveCamp)
{
}

/**
 * 璇锋眰瀵艰埅鍒版柊鎵嬫潙
 * 澶勭悊杩涘叆鏂版墜鏉戠殑璇锋眰
 */
void UDBAMainLobbyWidgetController::RequestNavigateToNewbieVillage()
{
}

/**
 * 璇锋眰瀵艰埅鍒扮粌涔犳ā寮? * 澶勭悊杩涘叆缁冧範妯″紡鐨勮姹? */
void UDBAMainLobbyWidgetController::RequestNavigateToPractice()
{
}

/**
 * 璇锋眰閫€鍑烘父鎴? * 澶勭悊閫€鍑烘父鎴忕殑璇锋眰
 */
void UDBAMainLobbyWidgetController::RequestExitGame()
{
}

