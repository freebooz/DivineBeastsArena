// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectViewModel.h"

#define LOCTEXT_NAMESPACE "DBAServerSelectViewModel"

bool UDBAServerSelectViewModel::CanConfirmSelection() const
{
	return OperationState != EDBAAsyncOperationState::InProgress
		&& Servers.ContainsByPredicate([this](const FDBAServerSelectItemViewData& Entry)
		{
			return Entry.bCanSelect && Entry.ServerId == SelectedServerId;
		});
}

void UDBAServerSelectViewModel::ApplyDirectory(const TArray<FDBAServerDirectoryEntry>& InServers, const FString& LastServerId)
{
	Servers.Reset(InServers.Num());
	for (const FDBAServerDirectoryEntry& Entry : InServers)
	{
		Servers.Add(MakeViewData(Entry, LastServerId));
	}

	if (!Servers.ContainsByPredicate([this](const FDBAServerSelectItemViewData& Entry) { return Entry.ServerId == SelectedServerId; }))
	{
		SelectedServerId.Reset();
	}
	if (SelectedServerId.IsEmpty())
	{
		if (const FDBAServerSelectItemViewData* LastServer = Servers.FindByPredicate([](const FDBAServerSelectItemViewData& Entry)
			{
				return Entry.bIsLastLoginServer && Entry.bCanSelect;
			}))
		{
			SelectedServerId = LastServer->ServerId;
		}
		else if (const FDBAServerSelectItemViewData* FirstAvailable = Servers.FindByPredicate([](const FDBAServerSelectItemViewData& Entry)
			{
				return Entry.bCanSelect;
			}))
		{
			SelectedServerId = FirstAvailable->ServerId;
		}
	}

	LastError = FDBAApiError();
	OperationState = EDBAAsyncOperationState::Succeeded;
	OnChanged.Broadcast();
}

void UDBAServerSelectViewModel::SetOperationState(const EDBAAsyncOperationState InOperationState)
{
	if (OperationState != InOperationState)
	{
		OperationState = InOperationState;
		OnChanged.Broadcast();
	}
}

void UDBAServerSelectViewModel::SetLastError(const FDBAApiError& InError)
{
	LastError = InError;
	OperationState = InError.Category == EDBANetworkErrorCategory::Timeout
		? EDBAAsyncOperationState::TimedOut
		: EDBAAsyncOperationState::Failed;
	OnChanged.Broadcast();
}

bool UDBAServerSelectViewModel::SelectServer(const FString& ServerId)
{
	const FDBAServerSelectItemViewData* Entry = Servers.FindByPredicate([&ServerId](const FDBAServerSelectItemViewData& Candidate)
	{
		return Candidate.ServerId == ServerId;
	});
	if (!Entry || !Entry->bCanSelect)
	{
		return false;
	}

	if (SelectedServerId != Entry->ServerId)
	{
		SelectedServerId = Entry->ServerId;
		OnChanged.Broadcast();
	}
	return true;
}

FDBAServerSelectItemViewData UDBAServerSelectViewModel::MakeViewData(const FDBAServerDirectoryEntry& Entry, const FString& LastServerId)
{
	FDBAServerSelectItemViewData ViewData;
	ViewData.ServerId = Entry.ServerId;
	ViewData.Name = Entry.Name;
	ViewData.RegionText = FText::Format(LOCTEXT("Region", "区域：{0}"), FText::FromString(Entry.Region));
	ViewData.PopulationText = FText::Format(LOCTEXT("Population", "人数：{0}"), FText::AsNumber(Entry.Population));
	ViewData.bRecommended = Entry.bRecommended;
	ViewData.bIsLastLoginServer = !LastServerId.IsEmpty() && Entry.ServerId == LastServerId;
	ViewData.bCanSelect = Entry.bCanSelect;

	switch (Entry.Status)
	{
	case EDBAServerDirectoryStatus::Online:
		ViewData.StatusText = LOCTEXT("Online", "在线");
		break;
	case EDBAServerDirectoryStatus::Busy:
		ViewData.StatusText = LOCTEXT("Busy", "繁忙");
		break;
	case EDBAServerDirectoryStatus::Full:
		ViewData.StatusText = LOCTEXT("Full", "已满");
		break;
	case EDBAServerDirectoryStatus::Maintenance:
		ViewData.StatusText = LOCTEXT("Maintenance", "维护中");
		break;
	default:
		ViewData.StatusText = LOCTEXT("Offline", "离线");
		break;
	}

	if (!Entry.bCanSelect)
	{
		ViewData.UnavailableReason = !Entry.MaintenanceMessage.IsEmpty()
			? Entry.MaintenanceMessage
			: FText::Format(LOCTEXT("Unavailable", "当前区服{0}，暂不可进入。"), ViewData.StatusText);
	}
	return ViewData;
}

#undef LOCTEXT_NAMESPACE
