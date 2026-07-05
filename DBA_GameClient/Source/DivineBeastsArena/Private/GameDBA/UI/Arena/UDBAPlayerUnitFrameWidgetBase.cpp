// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"

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
	, CachedCurrentXP(0.0f)
	, CachedMaxXP(100.0f)
	, CachedUltimateEnergy(0.0f)
	, CachedMaxUltimateEnergy(DBAConstants::MaxUltimateEnergy)
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
	UpdateHP(CachedCurrentHP, CachedMaxHP);
	UpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy);
	UpdateXP(CachedCurrentXP, CachedMaxXP);
	UpdateUltimateEnergyWithMax(CachedUltimateEnergy, CachedMaxUltimateEnergy);
	UpdateLevel(CurrentLevel);
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
	if (WidgetController)
	{
		WidgetController->OnHPUpdated.RemoveDynamic(this, &ThisClass::HandleControllerHPUpdated);
		WidgetController->OnEnergyUpdated.RemoveDynamic(this, &ThisClass::HandleControllerEnergyUpdated);
		WidgetController->OnLevelUpdated.RemoveDynamic(this, &ThisClass::HandleControllerLevelUpdated);
	}

	WidgetController = InController;

	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnHPUpdated.AddDynamic(this, &ThisClass::HandleControllerHPUpdated);
	WidgetController->OnEnergyUpdated.AddDynamic(this, &ThisClass::HandleControllerEnergyUpdated);
	WidgetController->OnLevelUpdated.AddDynamic(this, &ThisClass::HandleControllerLevelUpdated);

	UpdateHP(WidgetController->GetCurrentHP(), WidgetController->GetMaxHP());
	UpdateEnergy(WidgetController->GetCurrentEnergy(), WidgetController->GetMaxEnergy());
	UpdateLevel(WidgetController->GetCurrentLevel());
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerHPUpdated(float CurrentHP, float MaxHP)
{
	UpdateHP(CurrentHP, MaxHP);
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerEnergyUpdated(float CurrentEnergy, float MaxEnergy)
{
	UpdateEnergy(CurrentEnergy, MaxEnergy);
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerLevelUpdated(int32 Level)
{
	UpdateLevel(Level);
}

/**
 * 鏇存柊鐜╁鐢熷懡鍊? * 缂撳瓨褰撳墠鍊煎苟璁＄畻琛€鏉＄櫨鍒嗘瘮锛岄€氳繃 Blueprint 浜嬩欢鏇存柊鏄剧ず
 * @param InCachedCurrentHP 褰撳墠鐢熷懡鍊? * @param InCachedMaxHP 鏈€澶х敓鍛藉€? */
void UDBAPlayerUnitFrameWidgetBase::UpdateHP(float InCachedCurrentHP, float InCachedMaxHP)
{
	CachedCurrentHP = FMath::Max(0.0f, InCachedCurrentHP);
	CachedMaxHP = FMath::Max(0.0f, InCachedMaxHP);

	// 璁＄畻琛€鏉＄櫨鍒嗘瘮
	float Percentage = FMath::Clamp(CachedMaxHP > 0.0f ? CachedCurrentHP / CachedMaxHP : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateHP(CachedCurrentHP, CachedMaxHP, Percentage);

	// 鏇存柊鐢熷懡鏉idget
	if (HealthBar)
	{
		HealthBar->SetPercent(Percentage);
	}
}

/**
 * 鏇存柊鐜╁鑳介噺鍊? * 缂撳瓨褰撳墠鍊煎苟璁＄畻鑳介噺鏉＄櫨鍒嗘瘮锛岄€氳繃 Blueprint 浜嬩欢鏇存柊鏄剧ず
 * @param InCachedCurrentEnergy 褰撳墠鑳介噺鍊? * @param InCachedMaxEnergy 鏈€澶ц兘閲忓€? */
void UDBAPlayerUnitFrameWidgetBase::UpdateEnergy(float InCachedCurrentEnergy, float InCachedMaxEnergy)
{
	CachedCurrentEnergy = FMath::Max(0.0f, InCachedCurrentEnergy);
	CachedMaxEnergy = FMath::Max(0.0f, InCachedMaxEnergy);

	// 璁＄畻鑳介噺鏉＄櫨鍒嗘瘮
	float Percentage = FMath::Clamp(CachedMaxEnergy > 0.0f ? CachedCurrentEnergy / CachedMaxEnergy : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy, Percentage);

	// 鏇存柊鑳介噺鏉idget
	if (EnergyBar)
	{
		EnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateXP(float InCachedCurrentXP, float InCachedMaxXP)
{
	CachedCurrentXP = FMath::Max(0.0f, InCachedCurrentXP);
	CachedMaxXP = FMath::Max(0.0f, InCachedMaxXP);

	// 璁＄畻缁忛獙鏉＄櫨鍒嗘瘮
	float Percentage = FMath::Clamp(CachedMaxXP > 0.0f ? CachedCurrentXP / CachedMaxXP : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateXP(CachedCurrentXP, CachedMaxXP, Percentage);

	// 鏇存柊缁忛獙鏉idget
	if (XPBar)
	{
		XPBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateUltimateEnergy(float Energy)
{
	UpdateUltimateEnergyWithMax(Energy, DBAConstants::MaxUltimateEnergy);
}

void UDBAPlayerUnitFrameWidgetBase::UpdateUltimateEnergyWithMax(float Energy, float MaxEnergy)
{
	CachedMaxUltimateEnergy = FMath::Max(1.0f, MaxEnergy);
	CachedUltimateEnergy = FMath::Clamp(Energy, 0.0f, CachedMaxUltimateEnergy);

	float Percentage = CachedUltimateEnergy / CachedMaxUltimateEnergy;
	BP_OnUpdateUltimateEnergy(CachedUltimateEnergy, Percentage);

	// 鏇存柊缁堟瀬鑳介噺鏉idget
	if (UltimateEnergyBar)
	{
		UltimateEnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateLevel(int32 Level)
{
	CurrentLevel = FMath::Max(1, Level);
	BP_OnUpdateLevel(CurrentLevel);
}

void UDBAPlayerUnitFrameWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	const uint8 NormalizedFiveCamp = FMath::Clamp(
		FiveCamp,
		static_cast<uint8>(EDBAFiveCamp::None),
		static_cast<uint8>(EDBAFiveCamp::Center));

	BP_OnApplyFiveCampTheme(NormalizedFiveCamp);
}
