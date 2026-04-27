// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/Data/DBAInputConfigDataAsset.h"

void UDBAInputConfigDataAsset::GetMappingContextsForPlatform(EDBAInputPlatform Platform, TArray<FDBAInputMappingContextConfig>& OutConfigs) const
{
	OutConfigs.Reset();

	for (const FDBAInputMappingContextConfig& Config : MappingContexts)
	{
		// 匹配目标平台或通用平台
		if (Config.Platform == Platform || Config.Platform == EDBAInputPlatform::Universal)
		{
			OutConfigs.Add(Config);
		}
	}

	// 按优先级排序（降序）
	OutConfigs.Sort([](const FDBAInputMappingContextConfig& A, const FDBAInputMappingContextConfig& B)
	{
		return A.Priority > B.Priority;
	});
}
