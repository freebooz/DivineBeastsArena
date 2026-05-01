// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 曜鸣神鸡

#include "Animation/Zodiac/DBAZodiacAnimConfig_Rooster.h"

UDBAZodiacAnimConfig_Rooster::UDBAZodiacAnimConfig_Rooster()
{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Idle.AM_Rooster_Idle"));
	if (IdleFinder.Succeeded())
	{
		Idle_Montage = IdleFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Walk.AM_Rooster_Walk"));
	if (WalkFinder.Succeeded())
	{
		Walk_Montage = WalkFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Run.AM_Rooster_Run"));
	if (RunFinder.Succeeded())
	{
		Run_Montage = RunFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Attack.AM_Rooster_Attack"));
	if (AttackFinder.Succeeded())
	{
		Attack_Montage = AttackFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Passive.AM_Rooster_Passive"));
	if (PassiveFinder.Succeeded())
	{
		Passive_Montage = PassiveFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Q.AM_Rooster_Q"));
	if (QFinder.Succeeded())
	{
		Q_Montage = QFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_W.AM_Rooster_W"));
	if (WFinder.Succeeded())
	{
		W_Montage = WFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_E.AM_Rooster_E"));
	if (EFinder.Succeeded())
	{
		E_Montage = EFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_R.AM_Rooster_R"));
	if (RFinder.Succeeded())
	{
		R_Montage = RFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Hit.AM_Rooster_Hit"));
	if (HitFinder.Succeeded())
	{
		Hit_Montage = HitFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/Rooster/Montages/AM_Rooster_Death.AM_Rooster_Death"));
	if (DeathFinder.Succeeded())
	{
		Death_Montage = DeathFinder.Object;
	}
}
