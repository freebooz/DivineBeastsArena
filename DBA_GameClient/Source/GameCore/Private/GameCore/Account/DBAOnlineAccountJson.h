// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"

class FDBAOnlineAccountJson
{
public:
	static FString BuildLoginRequest(const FDBALoginRequest& Request);
	static FString BuildGuestLoginRequest(const FString& DeviceId, const FString& DeviceName, const FString& Platform);
	static FString BuildCreateCharacterRequest(const FDBACharacterCreateRequest& Request);
	static FString BuildSelectCharacterRequest(const FDBACharacterId& CharacterId);

	static bool ParseLoginResponse(const FString& Json, FDBALoginResponse& OutResponse, FString& OutError);
	static bool ParseCharacterListResponse(const FString& Json, TArray<FDBACharacterSummary>& OutCharacters, FString& OutError);
	static bool ParseCreateCharacterResponse(const FString& Json, FDBACharacterCreateResponse& OutResponse, FString& OutError);
	static bool ParseSelectCharacterResponse(const FString& Json, FDBACharacterId& OutCharacterId, FString& OutError);

	static EDBALoginType ParseLoginType(const FString& Value);
	static EDBAAccountStatus ParseAccountStatus(const FString& Value);
	static EDBAZodiac ParseZodiac(const FString& Value);
	static EDBAElement ParseElement(const FString& Value);
	static EDBAFiveCamp ParseFiveCamp(const FString& Value);

	static FString ToString(EDBALoginType Value);
	static FString ToString(EDBAAccountStatus Value);
	static FString ToString(EDBAZodiac Value);
	static FString ToString(EDBAElement Value);
	static FString ToString(EDBAFiveCamp Value);
};
