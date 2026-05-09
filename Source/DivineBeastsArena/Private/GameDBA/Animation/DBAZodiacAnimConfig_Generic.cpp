// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化生肖动画配置实现

#include "GameDBA/Animation/DBAZodiacAnimConfig_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

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

	if (AnimationType == IdleStr) return Idle_Montage.LoadSynchronous();
	if (AnimationType == WalkStr) return Walk_Montage.LoadSynchronous();
	if (AnimationType == RunStr) return Run_Montage.LoadSynchronous();
	if (AnimationType == AttackStr) return Attack_Montage.LoadSynchronous();
	if (AnimationType == PassiveStr) return Passive_Montage.LoadSynchronous();
	if (AnimationType == QStr) return Q_Montage.LoadSynchronous();
	if (AnimationType == WStr) return W_Montage.LoadSynchronous();
	if (AnimationType == EStr) return E_Montage.LoadSynchronous();
	if (AnimationType == RStr) return R_Montage.LoadSynchronous();
	if (AnimationType == HitStr) return Hit_Montage.LoadSynchronous();
	if (AnimationType == DeathStr) return Death_Montage.LoadSynchronous();

	UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacAnimConfig_Generic] 未找到动画类型: %s"), *AnimationType.ToString());
	return nullptr;
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
