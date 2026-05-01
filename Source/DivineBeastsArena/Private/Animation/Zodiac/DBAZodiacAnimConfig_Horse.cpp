// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 踏风天驹

#include "Animation/Zodiac/DBAZodiacAnimConfig_Horse.h"

UDBAZodiacAnimConfig_Horse::UDBAZodiacAnimConfig_Horse()
{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Idle.AM_Horse_Idle"));
	if (IdleFinder.Succeeded())
	{
		Idle_Montage = IdleFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Walk.AM_Horse_Walk"));
	if (WalkFinder.Succeeded())
	{
		Walk_Montage = WalkFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Run.AM_Horse_Run"));
	if (RunFinder.Succeeded())
	{
		Run_Montage = RunFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Attack.AM_Horse_Attack"));
	if (AttackFinder.Succeeded())
	{
		Attack_Montage = AttackFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Passive.AM_Horse_Passive"));
	if (PassiveFinder.Succeeded())
	{
		Passive_Montage = PassiveFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Q.AM_Horse_Q"));
	if (QFinder.Succeeded())
	{
		Q_Montage = QFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_W.AM_Horse_W"));
	if (WFinder.Succeeded())
	{
		W_Montage = WFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_E.AM_Horse_E"));
	if (EFinder.Succeeded())
	{
		E_Montage = EFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_R.AM_Horse_R"));
	if (RFinder.Succeeded())
	{
		R_Montage = RFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Hit.AM_Horse_Hit"));
	if (HitFinder.Succeeded())
	{
		Hit_Montage = HitFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/Horse/Montages/AM_Horse_Death.AM_Horse_Death"));
	if (DeathFinder.Succeeded())
	{
		Death_Montage = DeathFinder.Object;
	}
}
