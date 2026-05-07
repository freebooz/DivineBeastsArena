// Copyright FreeboozStudio. All Rights Reserved.
// 鐢熻倴鑳藉姏鍩虹被瀹炵幇 - 鎵€鏈夌敓鑲栫浉鍏宠兘鍔涚殑鍩虹被

#include "GameDBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"


UDBAZodiacAbilityBase::UDBAZodiacAbilityBase()
{
	// 榛樿鐢熻倴绫诲瀷涓洪紶锛堢敓鑲栭『搴忕涓€浣嶏級
	ZodiacType = EDBAZodiacType::Rat;
}


bool UDBAZodiacAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 璋冪敤鐖剁被妫€鏌ワ紙BlockTags銆佸喎鍗寸瓑锛?	// 鐢熻倴鎶€鑳介€氬父涓鸿鍔ㄦ妧鑳斤紝婵€娲婚€昏緫澶嶇敤鍩虹被
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

