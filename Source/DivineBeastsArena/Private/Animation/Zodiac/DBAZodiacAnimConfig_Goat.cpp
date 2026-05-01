// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 灵泽仙羊

#include "Animation/Zodiac/DBAZodiacAnimConfig_Goat.h"

UDBAZodiacAnimConfig_Goat::UDBAZodiacAnimConfig_Goat()
{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Idle.AM_Goat_Idle"));
	if (IdleFinder.Succeeded())
	{
		Idle_Montage = IdleFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Walk.AM_Goat_Walk"));
	if (WalkFinder.Succeeded())
	{
		Walk_Montage = WalkFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Run.AM_Goat_Run"));
	if (RunFinder.Succeeded())
	{
		Run_Montage = RunFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Attack.AM_Goat_Attack"));
	if (AttackFinder.Succeeded())
	{
		Attack_Montage = AttackFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Passive.AM_Goat_Passive"));
	if (PassiveFinder.Succeeded())
	{
		Passive_Montage = PassiveFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Q.AM_Goat_Q"));
	if (QFinder.Succeeded())
	{
		Q_Montage = QFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_W.AM_Goat_W"));
	if (WFinder.Succeeded())
	{
		W_Montage = WFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_E.AM_Goat_E"));
	if (EFinder.Succeeded())
	{
		E_Montage = EFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_R.AM_Goat_R"));
	if (RFinder.Succeeded())
	{
		R_Montage = RFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Hit.AM_Goat_Hit"));
	if (HitFinder.Succeeded())
	{
		Hit_Montage = HitFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/Goat/Montages/AM_Goat_Death.AM_Goat_Death"));
	if (DeathFinder.Succeeded())
	{
		Death_Montage = DeathFinder.Object;
	}
}
