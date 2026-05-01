// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 裂风虎君

#include "Animation/Zodiac/DBAZodiacAnimConfig_Tiger.h"

UDBAZodiacAnimConfig_Tiger::UDBAZodiacAnimConfig_Tiger()
{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Idle.AM_Tiger_Idle"));
	if (IdleFinder.Succeeded())
	{
		Idle_Montage = IdleFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Walk.AM_Tiger_Walk"));
	if (WalkFinder.Succeeded())
	{
		Walk_Montage = WalkFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Run.AM_Tiger_Run"));
	if (RunFinder.Succeeded())
	{
		Run_Montage = RunFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Attack.AM_Tiger_Attack"));
	if (AttackFinder.Succeeded())
	{
		Attack_Montage = AttackFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Passive.AM_Tiger_Passive"));
	if (PassiveFinder.Succeeded())
	{
		Passive_Montage = PassiveFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Q.AM_Tiger_Q"));
	if (QFinder.Succeeded())
	{
		Q_Montage = QFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_W.AM_Tiger_W"));
	if (WFinder.Succeeded())
	{
		W_Montage = WFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_E.AM_Tiger_E"));
	if (EFinder.Succeeded())
	{
		E_Montage = EFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_R.AM_Tiger_R"));
	if (RFinder.Succeeded())
	{
		R_Montage = RFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Hit.AM_Tiger_Hit"));
	if (HitFinder.Succeeded())
	{
		Hit_Montage = HitFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/Tiger/Montages/AM_Tiger_Death.AM_Tiger_Death"));
	if (DeathFinder.Succeeded())
	{
		Death_Montage = DeathFinder.Object;
	}
}
