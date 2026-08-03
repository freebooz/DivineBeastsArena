// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Presentation/VFX/Feedback/DBAFloatingDamageComponent.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

namespace
{
	FRotator ResolveBillboardRotation(const FVector& WorldLocation, APlayerController* PlayerController)
	{
		FVector CameraLocation = FVector::ZeroVector;
		FRotator CameraRotation = FRotator::ZeroRotator;
		if (PlayerController)
		{
			PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
			const FVector ToCamera = CameraLocation - WorldLocation;
			if (!ToCamera.IsNearlyZero())
			{
				return ToCamera.Rotation();
			}
		}

		return CameraRotation;
	}
}

UDBAFloatingDamageComponent::UDBAFloatingDamageComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
}

void UDBAFloatingDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存 PlayerController
	if (UWorld* World = GetWorld())
	{
		CachedPlayerController = World->GetFirstPlayerController();
	}

	// 预初始化对象池
	ActiveDamageEntries.Reserve(PoolSize);
	AvailableDamageEntries.Reserve(PoolSize);
}

void UDBAFloatingDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新所有活跃的伤害数字
	for (int32 i = ActiveDamageEntries.Num() - 1; i >= 0; --i)
	{
		FDBAFloatingDamageEntry& Entry = ActiveDamageEntries[i];
		UpdateDamageEntry(Entry, DeltaTime);

		// 更新 World 位置
		Entry.WorldLocation += Entry.Velocity * DeltaTime;

		// 时间衰减
		Entry.RemainingTime -= DeltaTime;
		if (Entry.RemainingTime <= 0.0f)
		{
			RecycleDamageEntry(Entry);
			ActiveDamageEntries.RemoveAtSwap(i);
		}
	}
}

void UDBAFloatingDamageComponent::SpawnDamageNumber(float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint)
{
	// 从对象池获取或创建新条目
	FDBAFloatingDamageEntry Entry;
	if (AvailableDamageEntries.Num() > 0)
	{
		Entry = AvailableDamageEntries.Pop(EAllowShrinking::No);
	}
	else if (ActiveDamageEntries.Num() < PoolSize)
	{
		// 允许超过池大小一点点
	}
	else
	{
		// 复用最旧的
		Entry = ActiveDamageEntries[0];
		ActiveDamageEntries.RemoveAt(0);
	}

	// 计算颜色
	FLinearColor DamageColor = FLinearColor::White;
	if (bIsCritical)
	{
		DamageColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 红色
	}
	else if (ElementValue > 0 && ElementValue <= 5)
	{
		// 根据元素类型设置颜色
		// EDBAElementType: None=0, Metal=1, Wood=2, Water=3, Fire=4, Earth=5
		switch (static_cast<EDBAElementType>(ElementValue))
		{
		case EDBAElementType::Fire:
			DamageColor = FLinearColor(1.0f, 0.27f, 0.0f, 1.0f);    // 橘红
			break;
		case EDBAElementType::Water:
			DamageColor = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);   // 冰蓝
			break;
		case EDBAElementType::Metal:
			DamageColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);   // 金黄
			break;
		case EDBAElementType::Earth:
			DamageColor = FLinearColor(0.55f, 0.27f, 0.07f, 1.0f); // 褐色
			break;
		case EDBAElementType::Wood:
			DamageColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);    // 绿色
			break;
		default:
			DamageColor = FLinearColor::White;
			break;
		}
	}

	// 初始化条目
	Entry = FDBAFloatingDamageEntry(ImpactPoint, Damage, DamageColor, bIsCritical);
	Entry.RemainingTime = DamageDuration;
	Entry.TotalTime = DamageDuration;

	SpawnDamageNumberEntry(Entry);
	ActiveDamageEntries.Add(Entry);
}

void UDBAFloatingDamageComponent::SetDamageNumberSystem(UNiagaraSystem* InDamageNumberSystem)
{
	DamageNumberSystem = InDamageNumberSystem;
}

void UDBAFloatingDamageComponent::ClearAllDamageNumbers()
{
	for (FDBAFloatingDamageEntry& Entry : ActiveDamageEntries)
	{
		RecycleDamageEntry(Entry);
	}
	ActiveDamageEntries.Empty();
}

void UDBAFloatingDamageComponent::SpawnDamageNumberEntry(FDBAFloatingDamageEntry& Entry)
{
	if (UWorld* World = GetWorld())
	{
		if (DamageNumberSystem)
		{
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				DamageNumberSystem,
				Entry.WorldLocation,
				FRotator::ZeroRotator,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true
			);

			if (NiagaraComp)
			{
				// 设置 User 参数
				NiagaraComp->SetVariableLinearColor(FName("DamageColor"), Entry.Color);
				NiagaraComp->SetVariableFloat(FName("DamageValue"), Entry.Damage);
				NiagaraComp->SetVariableBool(FName("bIsCritical"), Entry.bIsCritical);
				Entry.NiagaraComponent = NiagaraComp;
			}
			return;
		}

		UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(GetOwner());
		if (TextComp)
		{
			TextComp->RegisterComponentWithWorld(World);
			TextComp->SetWorldLocation(Entry.WorldLocation);
			TextComp->SetWorldRotation(ResolveBillboardRotation(Entry.WorldLocation, CachedPlayerController.Get()));
			TextComp->SetHorizontalAlignment(EHTA_Center);
			TextComp->SetVerticalAlignment(EVRTA_TextCenter);
			TextComp->SetTextRenderColor(Entry.Color.ToFColor(true));
			TextComp->SetWorldSize(Entry.bIsCritical ? 96.0f : 72.0f);
			TextComp->SetText(FText::AsNumber(FMath::RoundToInt(Entry.Damage)));
			Entry.TextComponent = TextComp;
		}
	}
}

void UDBAFloatingDamageComponent::UpdateDamageEntry(FDBAFloatingDamageEntry& Entry, float DeltaTime)
{
	// 鍚戜笂椋樺姩鐨勯€熷害浼氶€愭笎鍑忓皬
	Entry.Velocity.Z = FMath::Max(Entry.Velocity.Z - 200.0f * DeltaTime, 0.0f);
	if (Entry.TextComponent.IsValid())
	{
		Entry.TextComponent->SetWorldLocation(Entry.WorldLocation);
		if (!CachedPlayerController.IsValid())
		{
			if (UWorld* World = GetWorld())
			{
				CachedPlayerController = World->GetFirstPlayerController();
			}
		}
		Entry.TextComponent->SetWorldRotation(ResolveBillboardRotation(Entry.WorldLocation, CachedPlayerController.Get()));
		const float Alpha = Entry.TotalTime > 0.0f ? FMath::Clamp(Entry.RemainingTime / Entry.TotalTime, 0.0f, 1.0f) : 0.0f;
		FLinearColor FadedColor = Entry.Color;
		FadedColor.A = Alpha;
		Entry.TextComponent->SetTextRenderColor(FadedColor.ToFColor(true));
	}
}

void UDBAFloatingDamageComponent::RecycleDamageEntry(FDBAFloatingDamageEntry& Entry)
{
	if (Entry.NiagaraComponent.IsValid())
	{
		Entry.NiagaraComponent->Deactivate();
		Entry.NiagaraComponent = nullptr;
	}
	if (Entry.TextComponent.IsValid())
	{
		Entry.TextComponent->DestroyComponent();
		Entry.TextComponent = nullptr;
	}

	if (AvailableDamageEntries.Num() < PoolSize)
	{
		AvailableDamageEntries.Add(Entry);
	}
}
