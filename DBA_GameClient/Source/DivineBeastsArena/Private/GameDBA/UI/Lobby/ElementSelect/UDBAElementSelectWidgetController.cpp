// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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

