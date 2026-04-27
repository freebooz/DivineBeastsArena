// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MobaBase/GAS/DBAMobaAbilityGrantTypes.h"
#include "DBAMobaAbilitySystemComponentBase.generated.h"

class UDBAMobaAbilitySetData;
class UDBAMobaGameplayAbilityBase;

/**
 * MOBA 游戏 AbilitySystemComponent Base 层
 *
 * 职责：
 * - 管理 Ability / GameplayEffect / AttributeSet 的赋予与移除
 * - 处理 Authority / Prediction / Replication 边界
 * - 提供输入绑定到 Ability 的桥接机制
 * - 支持 AbilitySet 批量赋予 / 移除
 * - 提供 GameplayCue 事件桥接接口（Dedicated Server 不创建表现资源）
 */
UCLASS(ClassGroup = (AbilitySystem), meta = (BlueprintSpawnableComponent, DisplayName = "MOBA AbilitySystemComponent Base"))
class DIVINEBEASTSARENA_API UDBAMobaAbilitySystemComponentBase : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDBAMobaAbilitySystemComponentBase(const FObjectInitializer& ObjectInitializer);

	// ========== AbilitySystemComponent 接口实现 ==========

	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========== Ability 赋予 / 移除 ==========

	/**
	 * 赋予单个 Ability
	 * @param Config Ability 赋予配置
	 * @return Ability 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Ability")
	FDBAMobaAbilityGrantHandle GrantAbility(const FDBAMobaAbilityGrantConfig& Config);

	/**
	 * 移除单个 Ability
	 * @param Handle Ability 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Ability")
	void RemoveAbility(const FDBAMobaAbilityGrantHandle& Handle);

	/**
	 * 批量赋予 Ability
	 * @param Configs Ability 赋予配置列表
	 * @return Ability 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Ability")
	TArray<FDBAMobaAbilityGrantHandle> GrantAbilities(const TArray<FDBAMobaAbilityGrantConfig>& Configs);

	/**
	 * 批量移除 Ability
	 * @param Handles Ability 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Ability")
	void RemoveAbilities(const TArray<FDBAMobaAbilityGrantHandle>& Handles);

	// ========== GameplayEffect 赋予 / 移除 ==========

	/**
	 * 赋予单个 GameplayEffect
	 * @param Config GameplayEffect 赋予配置
	 * @return GameplayEffect 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Effect")
	FDBAMobaEffectGrantHandle GrantEffect(const FDBAMobaEffectGrantConfig& Config);

	/**
	 * 移除单个 GameplayEffect
	 * @param Handle GameplayEffect 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Effect")
	void RemoveEffect(const FDBAMobaEffectGrantHandle& Handle);

	/**
	 * 批量赋予 GameplayEffect
	 * @param Configs GameplayEffect 赋予配置列表
	 * @return GameplayEffect 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Effect")
	TArray<FDBAMobaEffectGrantHandle> GrantEffects(const TArray<FDBAMobaEffectGrantConfig>& Configs);

	/**
	 * 批量移除 GameplayEffect
	 * @param Handles GameplayEffect 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Effect")
	void RemoveEffects(const TArray<FDBAMobaEffectGrantHandle>& Handles);

	// ========== AttributeSet 赋予 / 移除 ==========

	/**
	 * 赋予单个 AttributeSet
	 * @param Config AttributeSet 赋予配置
	 * @return AttributeSet 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AttributeSet")
	FDBAMobaAttributeSetGrantHandle GrantAttributeSet(const FDBAMobaAttributeSetGrantConfig& Config);

	/**
	 * 移除单个 AttributeSet
	 * @param Handle AttributeSet 赋予句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AttributeSet")
	void RemoveAttributeSet(const FDBAMobaAttributeSetGrantHandle& Handle);

	/**
	 * 批量赋予 AttributeSet
	 * @param Configs AttributeSet 赋予配置列表
	 * @return AttributeSet 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AttributeSet")
	TArray<FDBAMobaAttributeSetGrantHandle> GrantAttributeSets(const TArray<FDBAMobaAttributeSetGrantConfig>& Configs);

	/**
	 * 批量移除 AttributeSet
	 * @param Handles AttributeSet 赋予句柄列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AttributeSet")
	void RemoveAttributeSets(const TArray<FDBAMobaAttributeSetGrantHandle>& Handles);

	// ========== AbilitySet 赋予 / 移除 ==========

	/**
	 * 赋予 AbilitySet（批量赋予 Ability / Effect / AttributeSet）
	 * @param AbilitySetData AbilitySet 数据资产
	 * @return AbilitySet 赋予结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AbilitySet")
	FDBAMobaAbilitySetGrantResult GrantAbilitySet(UDBAMobaAbilitySetData* AbilitySetData);

	/**
	 * 移除 AbilitySet（批量移除 Ability / Effect / AttributeSet）
	 * @param GrantResult AbilitySet 赋予结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|AbilitySet")
	void RemoveAbilitySet(const FDBAMobaAbilitySetGrantResult& GrantResult);

	// ========== 输入绑定 ==========

	/**
	 * 绑定输入到 Ability
	 * @param InputID 输入 ID
	 * @param InputTag 输入 Tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Input")
	void BindAbilityToInput(int32 InputID, FGameplayTag InputTag);

	/**
	 * 解绑输入
	 * @param InputID 输入 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Input")
	void UnbindAbilityFromInput(int32 InputID);

	/**
	 * 通过输入 Tag 激活 Ability
	 * @param InputTag 输入 Tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Input")
	void TryActivateAbilityByInputTag(FGameplayTag InputTag);

	// ========== 辅助函数 ==========

	/**
	 * 获取所有已赋予的 Ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Query")
	TArray<UDBAMobaGameplayAbilityBase*> GetAllGrantedAbilities() const;

	/**
	 * 通过 Ability Tag 查找 Ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Query")
	UDBAMobaGameplayAbilityBase* FindAbilityByTag(FGameplayTag AbilityTag) const;

private:
	/** 输入 ID 到 Ability Spec Handle 的映射 */
	TMap<int32, FGameplayAbilitySpecHandle> InputIDToAbilitySpecMap;

	/** 输入 Tag 到 Ability Spec Handle 的映射 */
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> InputTagToAbilitySpecMap;
};
