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
#include "GameplayCueNotify_Actor.h"
#include "DBACue_Base.generated.h"

class UDataTable;
class UParticleSystem;
class USoundBase;
struct FDBASkillDataRow;

UCLASS(Abstract)
class DIVINEBEASTSARENA_API ADBACue_Base : public AGameplayCueNotify_Actor
{
    GENERATED_BODY()
public:
    ADBACue_Base();

    virtual bool OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);
    virtual void OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);
    virtual void OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

protected:
    // Load skill data from DataTable
    void LoadSkillData();

    // Play VFX helper
    void PlayVFX(AActor* Target, const FGameplayCueParameters& Parameters, float Scale);

    // Play SFX helper
    void PlaySFX(AActor* Target, const FGameplayCueParameters& Parameters);

    /** 获取技能ID - 子类可重写 */
    virtual FName GetSkillId() const { return SkillId; }

    /** 技能ID (可在蓝图中配置) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
    FName SkillId = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Cue")
    float CueScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue|Fallback")
    TSoftObjectPtr<UParticleSystem> DefaultVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue|Fallback")
    TSoftObjectPtr<USoundBase> DefaultSFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue|Fallback")
    bool bPreferCueLocation = true;
};
