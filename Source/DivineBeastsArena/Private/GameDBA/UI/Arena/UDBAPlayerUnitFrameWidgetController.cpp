// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栫帺瀹跺崟鍏冩鎺у埗鍣? */
UDBAPlayerUnitFrameWidgetController::UDBAPlayerUnitFrameWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 鑾峰彇褰撳墠鐢熷懡鍊? * @return 褰撳墠 HP锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentHP() const
{
	return 850.0f;
}

/**
 * 鑾峰彇鏈€澶х敓鍛藉€? * @return 鏈€澶?HP锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxHP() const
{
	return 1000.0f;
}

/**
 * 鑾峰彇褰撳墠鑳介噺鍊? * @return 褰撳墠鑳介噺锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentEnergy() const
{
	return 70.0f;
}

/**
 * 鑾峰彇鏈€澶ц兘閲忓€? * @return 鏈€澶ц兘閲忥紙寰呬粠 GAS 灞炴€х郴缁熻幏鍙栵級
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxEnergy() const
{
	return 100.0f;
}

/**
 * 鑾峰彇褰撳墠绛夌骇
 * @return 鑻遍泟绛夌骇锛堝緟浠?GAS 灞炴€х郴缁熻幏鍙栵級
 */
int32 UDBAPlayerUnitFrameWidgetController::GetCurrentLevel() const
{
	return 12;
}

