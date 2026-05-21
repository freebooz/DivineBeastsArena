// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

UDBAFloatingDamageComponent::UDBAFloatingDamageComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
}

void UDBAFloatingDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缂撳瓨PlayerController
	if (UWorld* World = GetWorld())
	{
		CachedPlayerController = World->GetFirstPlayerController();
	}

	// 棰勫垵濮嬪寲瀵硅薄姹?	ActiveDamageEntries.Reserve(PoolSize);
	AvailableDamageEntries.Reserve(PoolSize);
}

void UDBAFloatingDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 鏇存柊鎵€鏈夋椿璺冪殑浼ゅ鏁板瓧
	for (int32 i = ActiveDamageEntries.Num() - 1; i >= 0; --i)
	{
		FDBAFloatingDamageEntry& Entry = ActiveDamageEntries[i];
		UpdateDamageEntry(Entry, DeltaTime);

		// 鏇存柊World浣嶇疆
		Entry.WorldLocation += Entry.Velocity * DeltaTime;

		// 鏃堕棿琛板噺
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
	// 浠庡璞℃睜鑾峰彇鎴栧垱寤烘柊鏉＄洰
	FDBAFloatingDamageEntry Entry;
	if (AvailableDamageEntries.Num() > 0)
	{
		Entry = AvailableDamageEntries.Pop(EAllowShrinking::No);
	}
	else if (ActiveDamageEntries.Num() < PoolSize)
	{
		// 鍏佽瓒呰繃姹犲ぇ灏忎竴鐐圭偣
	}
	else
	{
		// 澶嶇敤鏈€鏃х殑
		Entry = ActiveDamageEntries[0];
		ActiveDamageEntries.RemoveAt(0);
	}

	// 璁＄畻棰滆壊
	FLinearColor DamageColor = FLinearColor::White;
	if (bIsCritical)
	{
		DamageColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 绾㈣壊
	}
	else if (ElementValue > 0 && ElementValue <= 5)
	{
		// 鏍规嵁鍏冪礌绫诲瀷璁剧疆棰滆壊
		// EDBAElementType: None=0, Metal=1, Wood=2, Water=3, Fire=4, Earth=5
		switch (static_cast<EDBAElementType>(ElementValue))
		{
		case EDBAElementType::Fire:
			DamageColor = FLinearColor(1.0f, 0.27f, 0.0f, 1.0f);    // 姗欑孩
			break;
		case EDBAElementType::Water:
			DamageColor = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);   // 鍐拌摑
			break;
		case EDBAElementType::Metal:
			DamageColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);   // 閲戦粍
			break;
		case EDBAElementType::Earth:
			DamageColor = FLinearColor(0.55f, 0.27f, 0.07f, 1.0f); // 妫曡壊
			break;
		case EDBAElementType::Wood:
			DamageColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);    // 缁胯壊
			break;
		default:
			DamageColor = FLinearColor::White;
			break;
		}
	}

	// 鍒濆鍖栨潯鐩?	Entry = FDBAFloatingDamageEntry(ImpactPoint, Damage, DamageColor, bIsCritical);
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
				// 璁剧疆User鍙傛暟
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
			TextComp->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
			TextComp->SetHorizontalAlignment(EHTA_Center);
			TextComp->SetVerticalAlignment(EVRTA_TextCenter);
			TextComp->SetTextRenderColor(Entry.Color.ToFColor(true));
			TextComp->SetWorldSize(Entry.bIsCritical ? 48.0f : 36.0f);
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

