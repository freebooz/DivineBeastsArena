// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#include "GameDBA/GAS/Cues/DBACue_Base.h"
#include "GameDBA/Data/DBASkillDataRow.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
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

    UDataTable* LoadSkillDataTableIfAvailable(UObject* CallbackOwner)
    {
        static const TCHAR* CandidatePaths[] = {
            TEXT("/Game/DBA/Data/Skills/SkillDataTable.SkillDataTable"),
            TEXT("/Game/Data/Skills/SkillDataTable.SkillDataTable")
        };

        TArray<FSoftObjectPath> PathsToPreload;
        for (const TCHAR* CandidatePath : CandidatePaths)
        {
            const FSoftObjectPath SoftPath(CandidatePath);
            const FString PackageName = SoftPath.GetLongPackageName();
            if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
            {
                continue;
            }

            if (UDataTable* SkillTable = Cast<UDataTable>(SoftPath.ResolveObject()))
            {
                return SkillTable;
            }

            PathsToPreload.AddUnique(SoftPath);
        }

        DBAAsyncAssetLoader::RequestAsyncPreload(CallbackOwner, PathsToPreload);
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
        }
        else
        {
            TArray<FSoftObjectPath> Paths;
            DBAAsyncAssetLoader::AddPreloadPath(SkillData->VFXAsset, Paths);
            DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
        }
    }

    if (SkillData && !SkillData->SFXAsset.IsNull())
    {
        if (USoundBase* SFX = SkillData->SFXAsset.Get())
        {
            UGameplayStatics::PlaySoundAtLocation(Target, SFX, ResolveCueLocation(Target, Parameters, bPreferCueLocation));
        }
        else
        {
            TArray<FSoftObjectPath> Paths;
            DBAAsyncAssetLoader::AddPreloadPath(SkillData->SFXAsset, Paths);
            DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
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
    }
    else
    {
        TArray<FSoftObjectPath> Paths;
        DBAAsyncAssetLoader::AddPreloadPath(DefaultVFX, Paths);
        DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
    }
}

void ADBACue_Base::PlaySFX(AActor* Target, const FGameplayCueParameters& Parameters)
{
    if (USoundBase* SFX = DefaultSFX.Get())
    {
        UGameplayStatics::PlaySoundAtLocation(Target, SFX, ResolveCueLocation(Target, Parameters, bPreferCueLocation));
    }
    else
    {
        TArray<FSoftObjectPath> Paths;
        DBAAsyncAssetLoader::AddPreloadPath(DefaultSFX, Paths);
        DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
    }
}
