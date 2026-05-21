// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "GameBackendTypes.h"

DEFINE_LOG_CATEGORY(LogGameBackendClient);

class FGameBackendClientModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogGameBackendClient, Log, TEXT("GameBackendClient 模块已启动。"));
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogGameBackendClient, Log, TEXT("GameBackendClient 模块已关闭。"));
	}
};

IMPLEMENT_MODULE(FGameBackendClientModule, GameBackendClient)
