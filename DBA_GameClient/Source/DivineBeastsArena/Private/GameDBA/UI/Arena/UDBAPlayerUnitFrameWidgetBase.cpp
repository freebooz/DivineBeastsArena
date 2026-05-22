// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetBase.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栫帺瀹跺崟鍏冩 Widget
 * @param ObjectInitializer 瀵硅薄鍒濆鍖栧櫒
 */
UDBAPlayerUnitFrameWidgetBase::UDBAPlayerUnitFrameWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CachedCurrentHP(1000.0f)
	, CachedMaxHP(1000.0f)
	, CachedCurrentEnergy(100.0f)
	, CachedMaxEnergy(100.0f)
	, CurrentLevel(1)
{
}

/**
 * 鍘熺敓鏋勫缓鍥炶皟
 * 褰?Widget 鏋勫缓鍒板睆骞曟椂璋冪敤
 */
void UDBAPlayerUnitFrameWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 鍘熺敓閿€姣佸洖璋? * 褰?Widget 浠庡睆骞曠Щ闄ゆ椂璋冪敤锛岀敤浜庢竻鐞? */
void UDBAPlayerUnitFrameWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

/**
 * 鍘熺敓 Tick 鍥炶皟
 * 姣忓抚鏇存柊鐜╁鍗曞厓妗嗙姸鎬? * @param MyGeometry 褰撳墠 Widget 鍑犱綍淇℃伅
 * @param InDeltaTime 甯ч棿闅旀椂闂? */
void UDBAPlayerUnitFrameWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * Widget 琚縺娲绘椂鐨勫洖璋? * 褰撶帺瀹跺崟鍏冩鏄剧ず鏃惰皟鐢? */
void UDBAPlayerUnitFrameWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 琚仠鐢ㄦ椂鐨勫洖璋? * 褰撶帺瀹跺崟鍏冩闅愯棌鏃惰皟鐢? */
void UDBAPlayerUnitFrameWidgetBase::NativeOnDeactivated()
{
}

/**
 * 璁剧疆 Widget 鎺у埗鍣? * 灏嗘帶鍒跺櫒涓?Widget 鍏宠仈
 * @param InController 鐜╁鍗曞厓妗嗘帶鍒跺櫒鎸囬拡
 */
void UDBAPlayerUnitFrameWidgetBase::SetWidgetController(UDBAPlayerUnitFrameWidgetController* InController)
{
	WidgetController = InController;
}

/**
 * 鏇存柊鐜╁鐢熷懡鍊? * 缂撳瓨褰撳墠鍊煎苟璁＄畻琛€鏉＄櫨鍒嗘瘮锛岄€氳繃 Blueprint 浜嬩欢鏇存柊鏄剧ず
 * @param InCachedCurrentHP 褰撳墠鐢熷懡鍊? * @param InCachedMaxHP 鏈€澶х敓鍛藉€? */
void UDBAPlayerUnitFrameWidgetBase::UpdateHP(float InCachedCurrentHP, float InCachedMaxHP)
{
	CachedCurrentHP = InCachedCurrentHP;
	CachedMaxHP = InCachedMaxHP;

	// 璁＄畻琛€鏉＄櫨鍒嗘瘮
	float Percentage = CachedMaxHP > 0.0f ? CachedCurrentHP / CachedMaxHP : 0.0f;
	BP_OnUpdateHP(CachedCurrentHP, CachedMaxHP, Percentage);
}

/**
 * 鏇存柊鐜╁鑳介噺鍊? * 缂撳瓨褰撳墠鍊煎苟璁＄畻鑳介噺鏉＄櫨鍒嗘瘮锛岄€氳繃 Blueprint 浜嬩欢鏇存柊鏄剧ず
 * @param InCachedCurrentEnergy 褰撳墠鑳介噺鍊? * @param InCachedMaxEnergy 鏈€澶ц兘閲忓€? */
void UDBAPlayerUnitFrameWidgetBase::UpdateEnergy(float InCachedCurrentEnergy, float InCachedMaxEnergy)
{
	CachedCurrentEnergy = InCachedCurrentEnergy;
	CachedMaxEnergy = InCachedMaxEnergy;

	// 璁＄畻鑳介噺鏉＄櫨鍒嗘瘮
	float Percentage = CachedMaxEnergy > 0.0f ? CachedCurrentEnergy / CachedMaxEnergy : 0.0f;
	BP_OnUpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy, Percentage);

	// 鏇存柊鑳介噺鏉idget
	if (EnergyBar)
	{
		EnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateXP(float InCachedCurrentXP, float InCachedMaxXP)
{
	CachedCurrentXP = InCachedCurrentXP;
	CachedMaxXP = InCachedMaxXP;

	// 璁＄畻缁忛獙鏉＄櫨鍒嗘瘮
	float Percentage = CachedMaxXP > 0.0f ? CachedCurrentXP / CachedMaxXP : 0.0f;
	BP_OnUpdateXP(CachedCurrentXP, CachedMaxXP, Percentage);

	// 鏇存柊缁忛獙鏉idget
	if (XPBar)
	{
		XPBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateUltimateEnergy(float Energy)
{
	CachedUltimateEnergy = FMath::Clamp(Energy, 0.0f, 100.0f);

		float Percentage = CachedUltimateEnergy / 100.0f;
	BP_OnUpdateUltimateEnergy(CachedUltimateEnergy, Percentage);

	// 鏇存柊缁堟瀬鑳介噺鏉idget
	if (UltimateEnergyBar)
	{
		UltimateEnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateLevel(int32 Level)
{
	CurrentLevel = Level;
	BP_OnUpdateLevel(CurrentLevel);
}

void UDBAPlayerUnitFrameWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	BP_OnApplyFiveCampTheme(FiveCamp);
}


