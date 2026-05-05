// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Subsystems/DBASubsystemBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "DBAGameUIManager.generated.h"

class UDBAArenaHUDRootWidgetBase;
class UDBAMainLobbyWidgetBase;

/**
 * DBAGameUIManager
 *
 * 游戏UI状态管理器
 * 统一管理所有游戏界面的显示/隐藏和切换
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAGameUIManager : public UDBASubsystemBase
{
	GENERATED_BODY()

public:
	UDBAGameUIManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 显示主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowMainLobby();

	/** 隐藏主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideMainLobby();

	/** 显示战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowArenaHUD();

	/** 隐藏战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideArenaHUD();

	/** 清理所有UI */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ClearAllUI();

protected:
	/** 创建Lobby Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateMainLobbyWidget();

	/** 创建Arena HUD Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateArenaHUDWidget();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAMainLobbyWidgetBase> MainLobbyWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAArenaHUDRootWidgetBase> ArenaHUDWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAMainLobbyWidgetBase> MainLobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAArenaHUDRootWidgetBase> ArenaHUDWidgetClass;

private:
	bool bMainLobbyVisible = false;
	bool bArenaHUDVisible = false;
};