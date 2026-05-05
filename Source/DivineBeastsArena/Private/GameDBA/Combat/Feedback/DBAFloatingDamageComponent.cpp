// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextRenderComponent.h"

UDBAFloatingDamageComponent::UDBAFloatingDamageComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UDBAFloatingDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存PlayerController
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

		// 更新World位置
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
		Entry = AvailableDamageEntries.Pop(false);
	}
	else if (ActiveDamageEntries.Num() < PoolSize)
	{
		// 允许超过池大小一点点
	}
	else
	{
		// 复用最旧的
		Entry = ActiveDamageEntries.PopFront();
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
			DamageColor = FLinearColor(1.0f, 0.27f, 0.0f, 1.0f);    // 橙红
			break;
		case EDBAElementType::Water:
			DamageColor = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);   // 冰蓝
			break;
		case EDBAElementType::Metal:
			DamageColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);   // 金黄
			break;
		case EDBAElementType::Earth:
			DamageColor = FLinearColor(0.55f, 0.27f, 0.07f, 1.0f); // 棕色
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

void UDBAFloatingDamageComponent::SetDamageNumberSystem(TSubclassOf<UNiagaraSystem> InDamageNumberSystem)
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

void UDBAFloatingDamageComponent::SpawnDamageNumberEntry(const FDBAFloatingDamageEntry& Entry)
{
	if (!DamageNumberSystem.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DamageNumberSystem.Get(),
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
			// 设置User参数
			NiagaraComp->SetVariableLinearColor(FName("DamageColor"), Entry.Color);
			NiagaraComp->SetVariableFloat(FName("DamageValue"), Entry.Damage);
			NiagaraComp->SetVariableBool(FName("bIsCritical"), Entry.bIsCritical);
		}
	}
}

void UDBAFloatingDamageComponent::UpdateDamageEntry(FDBAFloatingDamageEntry& Entry, float DeltaTime)
{
	// 向上飘动的速度会逐渐减小
	Entry.Velocity.Z = FMath::Max(Entry.Velocity.Z - 200.0f * DeltaTime, 0.0f);
}

void UDBAFloatingDamageComponent::RecycleDamageEntry(FDBAFloatingDamageEntry& Entry)
{
	if (Entry.NiagaraComponent.IsValid())
	{
		Entry.NiagaraComponent->Deactivate();
		Entry.NiagaraComponent = nullptr;
	}

	if (AvailableDamageEntries.Num() < PoolSize)
	{
		AvailableDamageEntries.Add(Entry);
	}
}