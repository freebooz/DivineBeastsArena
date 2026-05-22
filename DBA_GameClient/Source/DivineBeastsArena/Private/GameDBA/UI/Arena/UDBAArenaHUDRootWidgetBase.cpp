// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖?HUD 鏍?Widget
 * @param ObjectInitializer 瀵硅薄鍒濆鍖栧櫒
 */
UDBAArenaHUDRootWidgetBase::UDBAArenaHUDRootWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsEditMode(false)
{
}

/**
 * 鍘熺敓鏋勫缓鍥炶皟
 * 褰?Widget 鏋勫缓鍒板睆骞曟椂璋冪敤锛岃繘琛屼簨浠剁粦瀹氱瓑鍒濆鍖栨搷浣? */
void UDBAArenaHUDRootWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 鍘熺敓閿€姣佸洖璋? * 褰?Widget 浠庡睆骞曠Щ闄ゆ椂璋冪敤锛岃繘琛屼簨浠惰В缁戠瓑娓呯悊鎿嶄綔
 */
void UDBAArenaHUDRootWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

/**
 * 鍘熺敓 Tick 鍥炶皟
 * 姣忓抚鏇存柊 HUD 鐘舵€? * @param MyGeometry 褰撳墠 Widget 鍑犱綍淇℃伅
 * @param InDeltaTime 甯ч棿闅旀椂闂? */
void UDBAArenaHUDRootWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * Widget 琚縺娲绘椂鐨勫洖璋? * 褰?HUD 鏄剧ず鏃惰皟鐢紝鍙噸鍐欎互鎵ц鏄剧ず閫昏緫
 */
void UDBAArenaHUDRootWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 琚仠鐢ㄦ椂鐨勫洖璋? * 褰?HUD 闅愯棌鏃惰皟鐢紝鍙噸鍐欎互鎵ц闅愯棌閫昏緫
 */
void UDBAArenaHUDRootWidgetBase::NativeOnDeactivated()
{
}

/**
 * 璁剧疆 Widget 鎺у埗鍣? * 灏嗘帶鍒跺櫒涓?Widget 鍏宠仈锛屼娇 Widget 鍙互鎺ユ敹鎺у埗鍣ㄦ洿鏂扮殑鏁版嵁
 * @param InController HUD Widget 鎺у埗鍣ㄦ寚閽? */
void UDBAArenaHUDRootWidgetBase::SetWidgetController(UDBAArenaHUDWidgetController* InController)
{
	WidgetController = InController;
}

/**
 * 璁剧疆 HUD 鍙鎬? * @param bVisible true 鏄剧ず HUD锛宖alse 闅愯棌 HUD
 */
void UDBAArenaHUDRootWidgetBase::SetHUDVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/**
 * 璁剧疆 HUD 缂栬緫妯″紡
 * @param bEditMode true 缂栬緫妯″紡锛宖alse 鏅€氭ā寮? * 缂栬緫妯″紡涓嬪彲鑳芥樉绀洪澶栬皟璇曚俊鎭? */
void UDBAArenaHUDRootWidgetBase::SetHUDEditMode(bool bEditMode)
{
	bIsEditMode = bEditMode;
}

/**
 * 搴旂敤浜斿ぇ闃佃惀涓婚
 * 鏍规嵁閫夋嫨鐨勯樀钀ユ敼鍙?HUD 鐨勯厤鑹插拰鏍峰紡
 * @param FiveCamp 闃佃惀绫诲瀷锛?-4 瀵瑰簲浜斿ぇ闃佃惀锛? */
void UDBAArenaHUDRootWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	BP_OnApplyFiveCampTheme(FiveCamp);
}

