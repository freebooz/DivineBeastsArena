// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBACharacterCreateWidgetController.generated.h"

class UDBAFrontendFlowSubsystem;

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetCharacterName(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetZodiac(EDBAZodiac InZodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetElement(EDBAElement InElement);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetFiveCamp(EDBAFiveCamp InFiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void Submit();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FDBACharacterCreateRequest PendingRequest;

	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
};
