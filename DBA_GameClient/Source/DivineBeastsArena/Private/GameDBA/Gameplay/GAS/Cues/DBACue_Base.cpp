// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#include "GameDBA/Gameplay/GAS/Cues/DBACue_Base.h"
#include "GameDBA/Data/Tables/DBASkillDataRow.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    FVector ResolveCueLocation(AActor* Target, const FGameplayCueParameters& Parameters, bool bPreferCueLocation)
    {
        if (bPreferCueLocation && !Parameters.Location.IsNearlyZero())
        {
            return FVector(Parameters.Location);
        }

        return Target ? Target->GetActorLocation() : FVector(Parameters.Location);
    }

    // P0-4 修复：原硬编码 CandidatePaths 数组已移除，改为优先使用 CueOwner->SkillDataTable 配置字段
    // 未配置时回退到 DBAConstants::DT_Skills 常量路径
    UDataTable* LoadSkillDataTableIfAvailable(ADBACue_Base* CueOwner)
    {
        TArray<FSoftObjectPath> CandidatePaths;

        if (CueOwner && !CueOwner->SkillDataTable.IsNull())
        {
            CandidatePaths.Add(CueOwner->SkillDataTable.ToSoftObjectPath());
        }
        else
        {
            // 回退到 DBAConstants 常量路径（统一路径管理，避免硬编码分歧）
            CandidatePaths.Add(FSoftObjectPath(DBAConstants::DT_Skills));
        }

        TArray<FSoftObjectPath> PathsToPreload;
        for (const FSoftObjectPath& SoftPath : CandidatePaths)
        {
            const FString PackageName = SoftPath.GetLongPackageName();
            if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
            {
                UE_LOG(LogDBACombat, Warning, TEXT("[ADBACue_Base] 技能数据表包不存在：%s"), *SoftPath.ToString());
                continue;
            }

            if (UDataTable* SkillTable = Cast<UDataTable>(SoftPath.ResolveObject()))
            {
                return SkillTable;
            }

            PathsToPreload.AddUnique(SoftPath);
        }

        if (PathsToPreload.Num() > 0 && CueOwner)
        {
            DBAAsyncAssetLoader::RequestAsyncPreload(CueOwner, PathsToPreload);
        }
        return nullptr;
    }
}

ADBACue_Base::ADBACue_Base()
{
}

bool ADBACue_Base::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
    FDBASkillDataRow* SkillData = nullptr;
    if (!SkillId.IsNone())
    {
        UDataTable* SkillTable = LoadSkillDataTableIfAvailable(this);
        if (SkillTable)
        {
            SkillData = SkillTable->FindRow<FDBASkillDataRow>(SkillId, TEXT("OnExecute"), false);
        }
    }

    if (SkillData && !SkillData->VFXAsset.IsNull())
    {
        if (UParticleSystem* VFX = SkillData->VFXAsset.Get())
        {
            FVector Location = ResolveCueLocation(Target, Parameters, bPreferCueLocation);
            FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
            UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, true);
            UE_LOG(LogDBAVFX, Log, TEXT("[ADBACue_Base] 播放技能 VFX：SkillId=%s，目标=%s，位置=%s"),
                *SkillId.ToString(), *GetNameSafe(Target), *Location.ToString());
        }
        else
        {
            TArray<FSoftObjectPath> Paths;
            DBAAsyncAssetLoader::AddPreloadPath(SkillData->VFXAsset, Paths);
            DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
            UE_LOG(LogDBAVFX, Warning, TEXT("[ADBACue_Base] 技能 VFX 未加载，已发起异步预加载：SkillId=%s，路径=%s"),
                *SkillId.ToString(), *SkillData->VFXAsset.ToSoftObjectPath().ToString());
        }
    }

    if (SkillData && !SkillData->SFXAsset.IsNull())
    {
        if (USoundBase* SFX = SkillData->SFXAsset.Get())
        {
            UGameplayStatics::PlaySoundAtLocation(Target, SFX, ResolveCueLocation(Target, Parameters, bPreferCueLocation));
            UE_LOG(LogDBASFX, Log, TEXT("[ADBACue_Base] 播放技能 SFX：SkillId=%s，目标=%s"),
                *SkillId.ToString(), *GetNameSafe(Target));
        }
        else
        {
            TArray<FSoftObjectPath> Paths;
            DBAAsyncAssetLoader::AddPreloadPath(SkillData->SFXAsset, Paths);
            DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
            UE_LOG(LogDBASFX, Warning, TEXT("[ADBACue_Base] 技能 SFX 未加载，已发起异步预加载：SkillId=%s，路径=%s"),
                *SkillId.ToString(), *SkillData->SFXAsset.ToSoftObjectPath().ToString());
        }
    }

    if (!SkillData)
    {
        PlayVFX(Target, Parameters, CueScale);
        PlaySFX(Target, Parameters);
    }

    return true;
}

void ADBACue_Base::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
    PlayVFX(Target, Parameters, CueScale * 0.7f);
}

void ADBACue_Base::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
}

void ADBACue_Base::LoadSkillData()
{
}

void ADBACue_Base::PlayVFX(AActor* Target, const FGameplayCueParameters& Parameters, float Scale)
{
    if (UParticleSystem* VFX = DefaultVFX.Get())
    {
        const FVector Location = ResolveCueLocation(Target, Parameters, bPreferCueLocation);
        const FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
        UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, FVector(Scale), true);
        UE_LOG(LogDBAVFX, Log, TEXT("[ADBACue_Base] 播放默认 VFX：目标=%s，位置=%s，缩放=%.2f"),
            *GetNameSafe(Target), *Location.ToString(), Scale);
    }
    else
    {
        TArray<FSoftObjectPath> Paths;
        DBAAsyncAssetLoader::AddPreloadPath(DefaultVFX, Paths);
        DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
        UE_LOG(LogDBAVFX, Warning, TEXT("[ADBACue_Base] 默认 VFX 未加载，已发起异步预加载：路径=%s"),
            *DefaultVFX.ToSoftObjectPath().ToString());
    }
}

void ADBACue_Base::PlaySFX(AActor* Target, const FGameplayCueParameters& Parameters)
{
    if (USoundBase* SFX = DefaultSFX.Get())
    {
        UGameplayStatics::PlaySoundAtLocation(Target, SFX, ResolveCueLocation(Target, Parameters, bPreferCueLocation));
        UE_LOG(LogDBASFX, Log, TEXT("[ADBACue_Base] 播放默认 SFX：目标=%s"), *GetNameSafe(Target));
    }
    else
    {
        TArray<FSoftObjectPath> Paths;
        DBAAsyncAssetLoader::AddPreloadPath(DefaultSFX, Paths);
        DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
        UE_LOG(LogDBASFX, Warning, TEXT("[ADBACue_Base] 默认 SFX 未加载，已发起异步预加载：路径=%s"),
            *DefaultSFX.ToSoftObjectPath().ToString());
    }
}
