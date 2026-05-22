// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栫帺瀹跺睘鎬ч粯璁ゅ€? */
UDBAArenaHUDWidgetController::UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentHP(1000.0f)
	, MaxHP(1000.0f)
	, CurrentEnergy(100.0f)
	, MaxEnergy(100.0f)
{
}

/**
 * 鏇存柊鐜╁鐢熷懡鍊? * @param InCurrentHP 褰撳墠鐢熷懡鍊? * @param InMaxHP 鏈€澶х敓鍛藉€? * 鏇存柊鍚庡箍鎾?OnPlayerHPChanged 浜嬩欢閫氱煡 UI 鏇存柊
 */
void UDBAArenaHUDWidgetController::UpdatePlayerHP(float InCurrentHP, float InMaxHP)
{
	CurrentHP = InCurrentHP;
	MaxHP = InMaxHP;
	OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP);
}

/**
 * 鏇存柊鐜╁鑳介噺鍊? * @param InCurrentEnergy 褰撳墠鑳介噺鍊? * @param InMaxEnergy 鏈€澶ц兘閲忓€? * 鏇存柊鍚庡箍鎾?OnPlayerEnergyChanged 浜嬩欢閫氱煡 UI 鏇存柊
 */
void UDBAArenaHUDWidgetController::UpdatePlayerEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	CurrentEnergy = InCurrentEnergy;
	MaxEnergy = InMaxEnergy;
	OnPlayerEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
}

/**
 * 鏇存柊缁堟瀬鑳介噺鍊? * @param InCurrentEnergy 褰撳墠缁堟瀬鑳介噺鍊? * @param InMaxEnergy 鏈€澶х粓鏋佽兘閲忓€硷紙鍥哄畾100锛? * 鏇存柊鍚庡箍鎾?OnUltimateEnergyChanged 浜嬩欢閫氱煡 UI 鏇存柊
 * 鐢ㄤ簬澶ф嫑鍏呰兘鏄剧ず鍜屽氨缁彁绀? */
void UDBAArenaHUDWidgetController::UpdateUltimateEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	OnUltimateEnergyChanged.Broadcast(InCurrentEnergy, InMaxEnergy);
}

