// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Controllers/Arena/UDBAPlayerUnitFrameWidgetController.h"
#include "GameFramework/PlayerController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栫帺瀹跺崟鍏冩鎺у埗鍣? */
UDBAPlayerUnitFrameWidgetController::UDBAPlayerUnitFrameWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAPlayerUnitFrameWidgetController::SetOwningPlayerController(APlayerController* InPlayerController)
{
	OwningPlayerController = InPlayerController;
}

APlayerController* UDBAPlayerUnitFrameWidgetController::GetOwningPlayerController() const
{
	return OwningPlayerController.Get();
}

void UDBAPlayerUnitFrameWidgetController::SetVitals(float InCurrentHP, float InMaxHP, float InCurrentEnergy, float InMaxEnergy)
{
	CurrentHP = FMath::Max(0.0f, InCurrentHP);
	MaxHP = FMath::Max(0.0f, InMaxHP);
	CurrentEnergy = FMath::Max(0.0f, InCurrentEnergy);
	MaxEnergy = FMath::Max(0.0f, InMaxEnergy);

	OnHPUpdated.Broadcast(CurrentHP, MaxHP);
	OnEnergyUpdated.Broadcast(CurrentEnergy, MaxEnergy);
}

void UDBAPlayerUnitFrameWidgetController::SetCurrentLevel(int32 InLevel)
{
	CurrentLevel = FMath::Max(1, InLevel);
	OnLevelUpdated.Broadcast(CurrentLevel);
}

/**
 * 鑾峰彇褰撳墠鐢熷懡鍊? * @return 褰撳墠 HP锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentHP() const
{
	return CurrentHP;
}

/**
 * 鑾峰彇鏈€澶х敓鍛藉€? * @return 鏈€澶?HP锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxHP() const
{
	return MaxHP;
}

/**
 * 鑾峰彇褰撳墠鑳介噺鍊? * @return 褰撳墠鑳介噺锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentEnergy() const
{
	return CurrentEnergy;
}

/**
 * 鑾峰彇鏈€澶ц兘閲忓€? * @return 鏈€澶ц兘閲忥紙寰呬粠 GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxEnergy() const
{
	return MaxEnergy;
}

/**
 * 鑾峰彇褰撳墠绛夌骇
 * @return 鑻遍泟绛夌骇锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
int32 UDBAPlayerUnitFrameWidgetController::GetCurrentLevel() const
{
	return CurrentLevel;
}
