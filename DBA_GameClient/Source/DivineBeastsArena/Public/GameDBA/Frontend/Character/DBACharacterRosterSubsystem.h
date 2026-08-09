// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "DBACharacterRosterSubsystem.generated.h"

/** 角色详情只在领域层保存；界面默认消费 FDBACharacterSummary。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Character")
	FDBACharacterSummary Summary;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Character")
	FString ServerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Character")
	FDBACharacterAppearance Appearance;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Character")
	bool bIsSelected = false;
};

using FDBACharacterRosterCompletion = TFunction<void(const FDBAOperationResult&)>;
using FDBACharacterDetailsCompletion = TFunction<void(const FDBAOperationResult&, const FDBACharacterDetails&)>;

DECLARE_MULTICAST_DELEGATE_OneParam(FDBAOnCharacterRosterChanged, const TArray<FDBACharacterSummary>&);

/**
 * 前台角色列表的唯一领域入口。
 * Widget 与 ViewModel 只能接收领域摘要，HTTP DTO/JSON 仅停留在本子系统实现中。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBACharacterRosterSubsystem final : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;

	void RefreshCharacterList(const FString& ServerId, FDBACharacterRosterCompletion Completion = {});
	const TArray<FDBACharacterSummary>& GetCachedCharacters() const { return CachedCharacters; }
	const FDBACharacterDetails* FindCachedCharacter(const FDBACharacterId& CharacterId) const;

	/**
	 * 提交角色创建意图。IdempotencyKey 由 Flow 为同一份 Draft 生成并在网络取消/重试时复用，
	 * Roster 只透传给服务端，不在 Widget 或日志中暴露该键。
	 */
	void CreateCharacter(const FDBACharacterCreateRequest& Request, const FDBACharacterAppearance& Appearance, const FString& IdempotencyKey, FDBACharacterDetailsCompletion Completion = {});

	/** 安全取消仍在等待的创建请求；服务端可能已收到请求，重试必须复用同一个幂等键。 */
	void CancelCreateCharacterRequest();

	/**
	 * 仅在服务端已成功创建后由 Flow 调用，写入前台选中摘要并广播缓存变化；
	 * 这不是进入世界的授权，也不会发送 select HTTP 请求。
	 */
	void SelectCreatedCharacterForFrontend(const FDBACharacterDetails& Character);
	void DeleteCharacter(const FDBACharacterId& CharacterId, FDBACharacterRosterCompletion Completion = {});
	void SelectCharacter(const FDBACharacterId& CharacterId, FDBACharacterDetailsCompletion Completion = {});
	void ClearSelectedCharacter();
	void ClearCache();

	FDBAOnCharacterRosterChanged& OnCharacterRosterChanged() { return CharacterRosterChanged; }

	/** 供自动化契约验证 DTO→Domain 边界；Widget 不得调用此方法。 */
	static bool ParseCharacterRosterJson(const FString& Json, TArray<FDBACharacterDetails>& OutDetails);
	static bool IsCacheScopeCurrent(
		uint64 ResponseRequestGeneration,
		uint64 ActiveRequestGeneration,
		const FString& ResponseAccountId,
		const FString& ActiveAccountId,
		const FString& ResponseServerId,
		const FString& ActiveServerId);

private:
	bool ResolveActiveCacheScope(FString& OutAccountId, FString& OutServerId) const;
	void ResetCacheForScope(const FString& AccountId, const FString& ServerId);
	bool IsRequestCurrent(uint64 RequestGeneration, const FString& AccountId, const FString& ServerId) const;
	/** 刷新后的选中回收：保留仍存在的角色，否则选择服务端标记角色、首项或清空。 */
	void ReconcileSelectionAfterRefresh();
	void PublishCache();
	void ApplySelectedCharacter(const FDBACharacterDetails& Character);
	void CompleteValidationFailure(const FString& Message, FDBACharacterRosterCompletion Completion) const;

	TArray<FDBACharacterSummary> CachedCharacters;
	TMap<FString, FDBACharacterDetails> CachedDetailsById;
	FString CachedAccountId;
	FString CachedServerId;
	uint64 RequestGeneration = 0;
	FGuid ActiveRosterRequestId;
	FGuid ActiveCreateRequestId;
	FDBAOnCharacterRosterChanged CharacterRosterChanged;
};
