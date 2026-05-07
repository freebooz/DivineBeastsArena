// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Loading/UDBALoadingWidgetController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栧姞杞界晫闈?Widget 鎺у埗鍣? */
UDBALoadingWidgetController::UDBALoadingWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LoadingProgress(0.0f)
{
}

/**
 * 璇锋眰鍔犺浇瀹屾垚
 * 褰撹祫婧愬姞杞藉畬鎴愭椂璋冪敤姝ゆ柟娉曢€氱煡 UI 闅愯棌鍔犺浇鐣岄潰
 * 骞挎挱 OnLoadingComplete 浜嬩欢
 */
void UDBALoadingWidgetController::RequestLoadComplete()
{
	OnLoadingComplete.Broadcast();
}

/**
 * 鑾峰彇鍔犺浇杩涘害
 * @return 褰撳墠鍔犺浇杩涘害锛?.0 - 1.0锛? */
float UDBALoadingWidgetController::GetLoadingProgress() const
{
	return LoadingProgress;
}

