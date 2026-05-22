// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 鐢熻倴瑙掕壊 - 澶滈殣鐏甸紶

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rat.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Rat::ADBAZodiacCharacter_Rat()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Fire;
	ZodiacType = EDBAZodiacType::Rat;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rat/ABP_Rat"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rat::BeginPlay()
{
	Super::BeginPlay();

	// 澶滈殣鐏甸紶瑙掕壊鐗瑰畾鍒濆鍖?
}


