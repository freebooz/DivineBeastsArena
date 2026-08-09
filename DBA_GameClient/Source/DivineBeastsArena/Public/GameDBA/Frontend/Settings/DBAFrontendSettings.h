// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/World.h"
#include "DBAFrontendSettings.generated.h"

class UDBAStartupVideoWidget;
class UDBAServerSelectScreenBase;
class UDBAAppearanceCatalogDataAsset;

/** 前台地图与角色槽位的唯一配置入口。地图均为软引用，不在启动阶段同步加载角色资源。 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "DBA Frontend"))
class DIVINEBEASTSARENA_API UDBAFrontendSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDBAFrontendSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> BootMap;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> FrontendMap;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (ClampMin = "1"))
	int32 MaxCharacterSlots = 0;

	/** 角色创建 Draft 使用的唯一外观选项目录；为空时只能提交生肖资产提供的默认外观。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UDBAAppearanceCatalogDataAsset> CharacterAppearanceCatalog;

	/** 仅允许 FlowSubsystem 读取；失败时必须无提示回退 Login。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Authentication")
	bool bAttemptAutoLogin = false;

	/** 是否允许使用项目已存在的游客账号入口；生产环境由配置明确控制。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Authentication")
	bool bEnableGuestLogin = true;

	/** 仅限非 Shipping 构建的开发入口总开关；Shipping 构建始终视为关闭。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Authentication")
	bool bAllowDevelopmentLogin = false;

	/** 默认是否将刷新令牌交由安全存储实现持久化；访问令牌始终只驻留内存。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Authentication")
	bool bRememberSessionByDefault = true;

	/** 启动页只加载此轻量 UI 软类，不得引用角色 Mesh、Niagara 或角色展示资产。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup")
	TSoftClassPtr<UDBAStartupVideoWidget> StartupScreenWidgetClass;

	/** 选服页仅在进入 ServerSelect 状态后按需加载；为空时使用轻量 C++ 回退页。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Frontend UI")
	TSoftClassPtr<UDBAServerSelectScreenBase> ServerSelectScreenWidgetClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup")
	FText StartupTitle;

	/** 版本探测为可选网络步骤；失败不会阻止进入前台登录页。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup")
	bool bCheckBackendVersionOnStartup = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup", meta = (ClampMin = "1.0", ClampMax = "15.0"))
	float StartupReachabilityTimeoutSeconds = 4.0f;
};
