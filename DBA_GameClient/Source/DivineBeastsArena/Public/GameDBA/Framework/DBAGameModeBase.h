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
#include "GameCore/Types/DBACommonEnums.h"
#include "GameMoba/Framework/DBAMobaGameModeBase.h"
#include "GameFramework/OnlineReplStructs.h"
#include "UObject/ObjectKey.h"
#include "DBAGameModeBase.generated.h"

class ADBACharacterPreviewActor;
class ADBALobbyTrainingMonster;
class APlayerController;
class AController;
class APawn;

UCLASS()
class DIVINEBEASTSARENA_API ADBAGameModeBase : public ADBAMobaGameModeBase
{
	GENERATED_BODY()

public:
	ADBAGameModeBase();

	static EDBAZodiac ResolveLobbyDisplayZodiac(const FString& Options, int32 JoinIndex);
	static FTransform GetLobbyDisplayTransform(int32 JoinIndex);

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

private:
	void SpawnOrUpdateLobbyDisplayForPlayer(APlayerController* PlayerController);
	void SpawnLobbyTrainingMonsters();
	UClass* ResolveLobbyPawnClass(EDBAZodiac Zodiac) const;

private:
	int32 NextLobbyJoinIndex = 0;
	TMap<TObjectKey<APlayerController>, int32> LobbyJoinIndices;
	TMap<TObjectKey<APlayerController>, EDBAZodiac> LobbyJoinZodiacs;
	TMap<TObjectKey<APlayerController>, TWeakObjectPtr<ADBACharacterPreviewActor>> LobbyDisplayActors;
	TArray<TWeakObjectPtr<ADBALobbyTrainingMonster>> LobbyTrainingMonsters;
};
