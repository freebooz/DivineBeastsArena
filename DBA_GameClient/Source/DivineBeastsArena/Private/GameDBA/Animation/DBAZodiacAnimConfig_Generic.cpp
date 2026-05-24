// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化生肖动画配置实现

#include "GameDBA/Animation/DBAZodiacAnimConfig_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"

UDBAZodiacAnimConfig_Generic::UDBAZodiacAnimConfig_Generic()
{
}

UAnimMontage* UDBAZodiacAnimConfig_Generic::GetAnimationByType(FName AnimationType) const
{
	static const FString IdleStr = TEXT("Idle");
	static const FString WalkStr = TEXT("Walk");
	static const FString RunStr = TEXT("Run");
	static const FString AttackStr = TEXT("Attack");
	static const FString PassiveStr = TEXT("Passive");
	static const FString QStr = TEXT("Q");
	static const FString WStr = TEXT("W");
	static const FString EStr = TEXT("E");
	static const FString RStr = TEXT("R");
	static const FString HitStr = TEXT("Hit");
	static const FString DeathStr = TEXT("Death");

	if (AnimationType == IdleStr) return Idle_Montage.Get();
	if (AnimationType == WalkStr) return Walk_Montage.Get();
	if (AnimationType == RunStr) return Run_Montage.Get();
	if (AnimationType == AttackStr) return Attack_Montage.Get();
	if (AnimationType == PassiveStr) return Passive_Montage.Get();
	if (AnimationType == QStr) return Q_Montage.Get();
	if (AnimationType == WStr) return W_Montage.Get();
	if (AnimationType == EStr) return E_Montage.Get();
	if (AnimationType == RStr) return R_Montage.Get();
	if (AnimationType == HitStr) return Hit_Montage.Get();
	if (AnimationType == DeathStr) return Death_Montage.Get();

	UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacAnimConfig_Generic] 未找到动画类型: %s"), *AnimationType.ToString());
	return nullptr;
}

void UDBAZodiacAnimConfig_Generic::PreloadAllAnimations()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(Idle_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Walk_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Run_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Attack_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Passive_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Q_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(W_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(E_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(R_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Hit_Montage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(Death_Montage, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

TMap<FName, TSoftObjectPtr<UAnimMontage>> UDBAZodiacAnimConfig_Generic::GetAllAnimations() const
{
	TMap<FName, TSoftObjectPtr<UAnimMontage>> Animations;
	Animations.Add(TEXT("Idle"), Idle_Montage);
	Animations.Add(TEXT("Walk"), Walk_Montage);
	Animations.Add(TEXT("Run"), Run_Montage);
	Animations.Add(TEXT("Attack"), Attack_Montage);
	Animations.Add(TEXT("Passive"), Passive_Montage);
	Animations.Add(TEXT("Q"), Q_Montage);
	Animations.Add(TEXT("W"), W_Montage);
	Animations.Add(TEXT("E"), E_Montage);
	Animations.Add(TEXT("R"), R_Montage);
	Animations.Add(TEXT("Hit"), Hit_Montage);
	Animations.Add(TEXT("Death"), Death_Montage);
	return Animations;
}
