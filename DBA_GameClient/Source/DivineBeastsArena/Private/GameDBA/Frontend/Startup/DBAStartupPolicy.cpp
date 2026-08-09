// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Startup/DBAStartupPolicy.h"

bool DBAStartupPolicy::IsFrontendMapConfigurationValid(const FSoftObjectPath& MapPath)
{
	return MapPath.IsValid() && !MapPath.GetLongPackageName().IsEmpty();
}

EDBAStartupCheckDisposition DBAStartupPolicy::ResolveBackendCheck(bool bConfigurationValid, bool bBackendReachable)
{
	// 网络不可用是可恢复状态，仍进入前台并展示离线提示；配置错误才阻断启动。
	return bConfigurationValid ? EDBAStartupCheckDisposition::TravelToFrontend : EDBAStartupCheckDisposition::FatalFailure;
}
