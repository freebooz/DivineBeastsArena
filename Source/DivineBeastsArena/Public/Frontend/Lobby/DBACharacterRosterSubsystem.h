// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBACharacterRosterSubsystem.generated.h"

/**
 * DBACharacterRosterSubsystem
 *
 * 角色名册子系统
 * 管理玩家已拥有的角色列表
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBACharacterRosterSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDBACharacterRosterSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 检查玩家是否拥有指定角色
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterRoster")
	bool HasCharacter(FName CharacterId) const;

	/**
	 * 获取玩家已拥有的角色列表
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterRoster")
	TArray<FName> GetOwnedCharacters() const;

	/**
	 * 添加角色到名册
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterRoster")
	void AddCharacter(FName CharacterId);

	/**
	 * 获取角色数量
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterRoster")
	int32 GetCharacterCount() const;

private:
	UPROPERTY()
	TArray<FName> OwnedCharacters;
};
