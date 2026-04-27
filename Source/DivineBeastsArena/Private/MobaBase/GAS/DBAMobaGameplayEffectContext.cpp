// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/GAS/DBAMobaGameplayEffectContext.h"
#include "Net/UnrealNetwork.h"

bool FDBAMobaGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// 序列化父类数据
	if (!FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	// 序列化自定义字段
	Ar << ElementCounterFactor;
	Ar << ChainLevel;
	Ar << ResonanceLevel;
	Ar << bIsCriticalHit;
	Ar << DamageIncreaseFactor;
	Ar << DamageReduceFactor;

	// 检查序列化过程是否有错误
	if (Ar.IsError())
	{
		bOutSuccess = false;
		return false;
	}

	bOutSuccess = true;
	return true;
}
