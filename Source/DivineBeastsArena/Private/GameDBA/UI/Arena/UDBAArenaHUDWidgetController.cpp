// Copyright Freebooz Games, Inc. All Rights Reserved.

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

