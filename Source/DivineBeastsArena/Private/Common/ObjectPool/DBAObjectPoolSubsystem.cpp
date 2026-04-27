// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/ObjectPool/DBAObjectPoolSubsystem.h"
#include "Common/ObjectPool/DBAObjectPoolTypes.h"
#include "Engine/World.h"
#include "Misc/App.h"

UDBAObjectPoolSubsystem::UDBAObjectPoolSubsystem()
{
}

void UDBAObjectPoolSubsystem::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LogSubsystemInfo(TEXT("对象池子系统初始化"));

	// 当前阶段不预注册任何池
	// 等待后续系统（Projectile / UI / Audio）注册各自的池

	bIsInitialized = true;
}

void UDBAObjectPoolSubsystem::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("对象池子系统反初始化"));

	// 清空所有池
	ClearAllPools();

	Super::OnSubsystemDeinitialize();
}

bool UDBAObjectPoolSubsystem::IsSupportedInCurrentEnvironment() const
{
	// 对象池子系统在所有环境都支持
	// 但具体的池可能在某些环境不创建（例如 UI 池在 Dedicated Server 不创建）
	return true;
}

bool UDBAObjectPoolSubsystem::RegisterObjectPool(const FDBAObjectPoolConfig& Config)
{
	if (!EnsureGameThread(TEXT("RegisterObjectPool")))
	{
		return false;
	}

	if (Config.PoolType == EDBAObjectPoolType::None)
	{
		LogSubsystemError(TEXT("注册对象池失败：池类型为 None"));
		return false;
	}

	if (!Config.ObjectClass)
	{
		LogSubsystemError(FString::Printf(TEXT("注册对象池失败：池类型 %d 的对象类为空"), static_cast<uint8>(Config.PoolType)));
		return false;
	}

	// 检查是否在 Dedicated Server
	const bool bIsDedicatedServer = GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer;
	if (bIsDedicatedServer && !Config.bCreateOnDedicatedServer)
	{
		LogSubsystemInfo(FString::Printf(TEXT("跳过注册对象池：池类型 %d 在 Dedicated Server 不创建"), static_cast<uint8>(Config.PoolType)));
		return false;
	}

	// 检查是否已存在
	if (ObjectPools.Contains(Config.PoolType))
	{
		LogSubsystemWarning(FString::Printf(TEXT("对象池已存在：池类型 %d，将覆盖"), static_cast<uint8>(Config.PoolType)));
		UnregisterObjectPool(Config.PoolType);
	}

	// 创建池数据
	FObjectPoolData PoolData;
	PoolData.Config = Config;
	PoolData.Stats.PoolType = Config.PoolType;

	// 应用 Android 池大小缩放
	if (IsAndroidPlatform())
	{
		PoolData.Config.InitialPoolSize = ApplyAndroidPoolSizeScale(Config.InitialPoolSize, Config.AndroidPoolSizeScale);
		PoolData.Config.MaxPoolSize = ApplyAndroidPoolSizeScale(Config.MaxPoolSize, Config.AndroidPoolSizeScale);
		LogSubsystemInfo(FString::Printf(TEXT("Android 平台池大小缩放：池类型 %d，初始大小 %d -> %d，最大大小 %d -> %d"),
			static_cast<uint8>(Config.PoolType),
			Config.InitialPoolSize, PoolData.Config.InitialPoolSize,
			Config.MaxPoolSize, PoolData.Config.MaxPoolSize));
	}

	// 预分配对象
	PoolData.AvailableObjects.Reserve(PoolData.Config.InitialPoolSize);
	for (int32 i = 0; i < PoolData.Config.InitialPoolSize; ++i)
	{
		UObject* NewObject = CreateNewObject(PoolData);
		if (NewObject)
		{
			PoolData.AvailableObjects.Add(NewObject);
			PoolData.Stats.TotalObjects++;
			PoolData.Stats.AvailableObjects++;
		}
	}

	// 添加到池列表
	ObjectPools.Add(Config.PoolType, PoolData);

	LogSubsystemInfo(FString::Printf(TEXT("注册对象池成功：池类型 %d，初始大小 %d，最大大小 %d"),
		static_cast<uint8>(Config.PoolType),
		PoolData.Config.InitialPoolSize,
		PoolData.Config.MaxPoolSize));

	return true;
}

void UDBAObjectPoolSubsystem::UnregisterObjectPool(EDBAObjectPoolType PoolType)
{
	if (!EnsureGameThread(TEXT("UnregisterObjectPool")))
	{
		return;
	}

	if (!ObjectPools.Contains(PoolType))
	{
		LogSubsystemWarning(FString::Printf(TEXT("注销对象池失败：池类型 %d 不存在"), static_cast<uint8>(PoolType)));
		return;
	}

	// 清空池
	ClearPool(PoolType);

	// 移除池
	ObjectPools.Remove(PoolType);

	LogSubsystemInfo(FString::Printf(TEXT("注销对象池成功：池类型 %d"), static_cast<uint8>(PoolType)));
}

UObject* UDBAObjectPoolSubsystem::AcquireObject(EDBAObjectPoolType PoolType)
{
	if (!EnsureGameThread(TEXT("AcquireObject")))
	{
		return nullptr;
	}

	FObjectPoolData* PoolData = ObjectPools.Find(PoolType);
	if (!PoolData)
	{
		LogSubsystemError(FString::Printf(TEXT("获取对象失败：池类型 %d 不存在"), static_cast<uint8>(PoolType)));
		return nullptr;
	}

	UObject* Object = nullptr;

	// 尝试从可用列表获取
	if (PoolData->AvailableObjects.Num() > 0)
	{
		Object = PoolData->AvailableObjects.Pop();
		PoolData->Stats.AvailableObjects--;
	}
	// 尝试动态创建
	else if (PoolData->Config.bAllowDynamicExpansion)
	{
		// 检查是否超过最大池大小
		if (PoolData->Config.MaxPoolSize > 0 && PoolData->Stats.TotalObjects >= PoolData->Config.MaxPoolSize)
		{
			LogSubsystemWarning(FString::Printf(TEXT("获取对象失败：池类型 %d 已达到最大大小 %d"),
				static_cast<uint8>(PoolType),
				PoolData->Config.MaxPoolSize));
			return nullptr;
		}

		Object = CreateNewObject(*PoolData);
		if (Object)
		{
			PoolData->Stats.TotalObjects++;
			PoolData->Stats.TotalDynamicCreateCount++;
			LogSubsystemInfo(FString::Printf(TEXT("动态创建对象：池类型 %d，当前总数 %d"),
				static_cast<uint8>(PoolType),
				PoolData->Stats.TotalObjects));
		}
	}
	else
	{
		LogSubsystemWarning(FString::Printf(TEXT("获取对象失败：池类型 %d 无可用对象且不允许动态扩展"), static_cast<uint8>(PoolType)));
		return nullptr;
	}

	if (!Object)
	{
		LogSubsystemError(FString::Printf(TEXT("获取对象失败：池类型 %d 创建对象失败"), static_cast<uint8>(PoolType)));
		return nullptr;
	}

	// 添加到使用中列表
	PoolData->ActiveObjects.Add(Object);
	PoolData->Stats.ActiveObjects++;
	PoolData->Stats.TotalAcquireCount++;

	// 调用激活回调
	if (Object->Implements<UDBAObjectPoolSubsystem>())
	{
		IDBAPoolableObject::Execute_OnAcquiredFromPool(Object);
	}

	return Object;
}

bool UDBAObjectPoolSubsystem::ReturnObject(EDBAObjectPoolType PoolType, UObject* Object)
{
	if (!EnsureGameThread(TEXT("ReturnObject")))
	{
		return false;
	}

	if (!IsObjectValid(Object))
	{
		LogSubsystemError(TEXT("归还对象失败：对象无效"));
		return false;
	}

	FObjectPoolData* PoolData = ObjectPools.Find(PoolType);
	if (!PoolData)
	{
		LogSubsystemError(FString::Printf(TEXT("归还对象失败：池类型 %d 不存在"), static_cast<uint8>(PoolType)));
		return false;
	}

	// 检查对象是否在使用中列表
	const int32 ActiveIndex = PoolData->ActiveObjects.Find(Object);
	if (ActiveIndex == INDEX_NONE)
	{
		LogSubsystemWarning(FString::Printf(TEXT("归还对象失败：对象不在池类型 %d 的使用中列表"), static_cast<uint8>(PoolType)));
		return false;
	}

	// 检查对象是否可以归还
	if (Object->Implements<UDBAObjectPoolSubsystem>())
	{
		if (!IDBAPoolableObject::Execute_CanReturnToPool(Object))
		{
			LogSubsystemWarning(FString::Printf(TEXT("归还对象失败：对象不允许归还到池类型 %d"), static_cast<uint8>(PoolType)));
			return false;
		}
	}

	// 从使用中列表移除
	PoolData->ActiveObjects.RemoveAtSwap(ActiveIndex);
	PoolData->Stats.ActiveObjects--;

	// 调用归还回调
	if (Object->Implements<UDBAObjectPoolSubsystem>())
	{
		IDBAPoolableObject::Execute_OnReturnedToPool(Object);

		// 重置对象
		if (PoolData->Config.bResetOnReturn)
		{
			IDBAPoolableObject::Execute_ResetPoolableObject(Object);
		}
	}

	// 添加到可用列表
	PoolData->AvailableObjects.Add(Object);
	PoolData->Stats.AvailableObjects++;
	PoolData->Stats.TotalReturnCount++;

	return true;
}

bool UDBAObjectPoolSubsystem::GetPoolStats(EDBAObjectPoolType PoolType, FDBAObjectPoolStats& OutStats) const
{
	const FObjectPoolData* PoolData = ObjectPools.Find(PoolType);
	if (!PoolData)
	{
		return false;
	}

	OutStats = PoolData->Stats;
	return true;
}

void UDBAObjectPoolSubsystem::ClearPool(EDBAObjectPoolType PoolType)
{
	if (!EnsureGameThread(TEXT("ClearPool")))
	{
		return;
	}

	FObjectPoolData* PoolData = ObjectPools.Find(PoolType);
	if (!PoolData)
	{
		LogSubsystemWarning(FString::Printf(TEXT("清空对象池失败：池类型 %d 不存在"), static_cast<uint8>(PoolType)));
		return;
	}

	// 清空可用列表
	PoolData->AvailableObjects.Empty();

	// 清空使用中列表
	PoolData->ActiveObjects.Empty();

	// 重置统计信息
	PoolData->Stats.TotalObjects = 0;
	PoolData->Stats.AvailableObjects = 0;
	PoolData->Stats.ActiveObjects = 0;

	LogSubsystemInfo(FString::Printf(TEXT("清空对象池成功：池类型 %d"), static_cast<uint8>(PoolType)));
}

void UDBAObjectPoolSubsystem::ClearAllPools()
{
	if (!EnsureGameThread(TEXT("ClearAllPools")))
	{
		return;
	}

	for (auto& Pair : ObjectPools)
	{
		ClearPool(Pair.Key);
	}

	LogSubsystemInfo(TEXT("清空所有对象池成功"));
}

void UDBAObjectPoolSubsystem::WarmupPool(EDBAObjectPoolType PoolType)
{
	if (!EnsureGameThread(TEXT("WarmupPool")))
	{
		return;
	}

	FObjectPoolData* PoolData = ObjectPools.Find(PoolType);
	if (!PoolData)
	{
		LogSubsystemWarning(FString::Printf(TEXT("预热对象池失败：池类型 %d 不存在"), static_cast<uint8>(PoolType)));
		return;
	}

	// 如果可用对象数量已达到初始大小，无需预热
	if (PoolData->AvailableObjects.Num() >= PoolData->Config.InitialPoolSize)
	{
		return;
	}

	// 创建对象直到达到初始大小
	const int32 TargetCount = PoolData->Config.InitialPoolSize - PoolData->AvailableObjects.Num();
	for (int32 i = 0; i < TargetCount; ++i)
	{
		UObject* NewObject = CreateNewObject(*PoolData);
		if (NewObject)
		{
			PoolData->AvailableObjects.Add(NewObject);
			PoolData->Stats.TotalObjects++;
			PoolData->Stats.AvailableObjects++;
		}
	}

	LogSubsystemInfo(FString::Printf(TEXT("预热对象池成功：池类型 %d，创建 %d 个对象"),
		static_cast<uint8>(PoolType),
		TargetCount));
}

void UDBAObjectPoolSubsystem::WarmupAllPools()
{
	if (!EnsureGameThread(TEXT("WarmupAllPools")))
	{
		return;
	}

	for (auto& Pair : ObjectPools)
	{
		WarmupPool(Pair.Key);
	}

	LogSubsystemInfo(TEXT("预热所有对象池成功"));
}

bool UDBAObjectPoolSubsystem::HasPool(EDBAObjectPoolType PoolType) const
{
	return ObjectPools.Contains(PoolType);
}

void UDBAObjectPoolSubsystem::GetAllPoolTypes(TArray<EDBAObjectPoolType>& OutPoolTypes) const
{
	ObjectPools.GetKeys(OutPoolTypes);
}

UObject* UDBAObjectPoolSubsystem::CreateNewObject(FObjectPoolData& PoolData)
{
	if (!PoolData.Config.ObjectClass)
	{
		LogSubsystemError(TEXT("创建对象失败：对象类为空"));
		return nullptr;
	}

	// 创建对象
	UClass* ObjectClass = PoolData.Config.ObjectClass;
	UObject* NewObject = ::NewObject<UObject>(this, ObjectClass);
	if (!NewObject)
	{
		LogSubsystemError(FString::Printf(TEXT("创建对象失败：无法创建类 %s 的实例"),
			*PoolData.Config.ObjectClass->GetName()));
		return nullptr;
	}

	// 检查对象是否实现了池化接口
	if (!NewObject->Implements<UDBAObjectPoolSubsystem>())
	{
		LogSubsystemWarning(FString::Printf(TEXT("创建对象警告：对象类 %s 未实现 IDBAPoolableObject 接口"),
			*PoolData.Config.ObjectClass->GetName()));
	}

	return NewObject;
}

bool UDBAObjectPoolSubsystem::IsObjectValid(UObject* Object) const
{
	return Object && IsValid(Object);
}

int32 UDBAObjectPoolSubsystem::ApplyAndroidPoolSizeScale(int32 OriginalSize, float Scale) const
{
	if (OriginalSize <= 0)
	{
		return 0;
	}

	return FMath::Max(1, static_cast<int32>(OriginalSize * Scale));
}

bool UDBAObjectPoolSubsystem::IsAndroidPlatform() const
{
#if PLATFORM_ANDROID
	return true;
#else
	return false;
#endif
}
