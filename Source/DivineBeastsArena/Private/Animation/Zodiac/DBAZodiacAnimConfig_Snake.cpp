// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 玄花灵蛇

#include "Animation/Zodiac/DBAZodiacAnimConfig_Snake.h"

UDBAZodiacAnimConfig_Snake::UDBAZodiacAnimConfig_Snake()
{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Idle.AM_Snake_Idle"));
	if (IdleFinder.Succeeded())
	{
		Idle_Montage = IdleFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Walk.AM_Snake_Walk"));
	if (WalkFinder.Succeeded())
	{
		Walk_Montage = WalkFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Run.AM_Snake_Run"));
	if (RunFinder.Succeeded())
	{
		Run_Montage = RunFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Attack.AM_Snake_Attack"));
	if (AttackFinder.Succeeded())
	{
		Attack_Montage = AttackFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Passive.AM_Snake_Passive"));
	if (PassiveFinder.Succeeded())
	{
		Passive_Montage = PassiveFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Q.AM_Snake_Q"));
	if (QFinder.Succeeded())
	{
		Q_Montage = QFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_W.AM_Snake_W"));
	if (WFinder.Succeeded())
	{
		W_Montage = WFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_E.AM_Snake_E"));
	if (EFinder.Succeeded())
	{
		E_Montage = EFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_R.AM_Snake_R"));
	if (RFinder.Succeeded())
	{
		R_Montage = RFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Hit.AM_Snake_Hit"));
	if (HitFinder.Succeeded())
	{
		Hit_Montage = HitFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/Snake/Montages/AM_Snake_Death.AM_Snake_Death"));
	if (DeathFinder.Succeeded())
	{
		Death_Montage = DeathFinder.Object;
	}
}
