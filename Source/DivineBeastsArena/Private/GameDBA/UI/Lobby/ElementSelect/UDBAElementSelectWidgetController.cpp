// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/ElementSelect/UDBAElementSelectWidgetController.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栧厓绱犻€夋嫨 Widget 鎺у埗鍣? */
UDBAElementSelectWidgetController::UDBAElementSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 纭鍏冪礌閫夋嫨
 * 鐢ㄦ埛閫夋嫨鍏冪礌鍚庤皟鐢ㄦ鏂规硶纭閫夋嫨
 * @param Element 閫夋嫨鐨勫厓绱犵被鍨? */
void UDBAElementSelectWidgetController::ConfirmElementSelection(EDBAElement Element)
{
	HandleElementConfirmed(true, Element);
}

/**
 * 璇锋眰杩斿洖涓婁竴椤? * 鐢ㄤ簬鍙栨秷褰撳墠閫夋嫨骞惰繑鍥炰笂涓€鐣岄潰
 */
void UDBAElementSelectWidgetController::RequestBack()
{
}

/**
 * 澶勭悊鍏冪礌纭鍥炶皟
 * @param bSuccess 鏄惁鎴愬姛纭
 * @param Element 纭鐨勫厓绱犵被鍨? * 鎴愬姛鏃跺箍鎾?OnElementConfirmed 浜嬩欢
 */
void UDBAElementSelectWidgetController::HandleElementConfirmed(bool bSuccess, EDBAElement Element)
{
	if (bSuccess)
	{
		OnElementConfirmed.Broadcast(Element);
	}
}

