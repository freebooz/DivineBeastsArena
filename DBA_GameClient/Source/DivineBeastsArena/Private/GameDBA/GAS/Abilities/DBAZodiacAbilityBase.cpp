// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

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

