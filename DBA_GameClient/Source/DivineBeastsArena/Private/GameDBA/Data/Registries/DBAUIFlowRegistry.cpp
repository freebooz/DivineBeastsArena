// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Data/Registries/DBAUIFlowRegistry.h"

#include "Misc/PackageName.h"

namespace
{
	FSoftObjectPath NormalizeWidgetClassPath(const FSoftObjectPath& ConfiguredPath)
	{
		if (ConfiguredPath.IsNull())
		{
			return ConfiguredPath;
		}

		const FString PathString = ConfiguredPath.ToString();
		if (PathString.Contains(TEXT(".")))
		{
			return ConfiguredPath;
		}

		const FString AssetName = FPackageName::GetShortName(PathString);
		return FSoftObjectPath(FString::Printf(TEXT("%s.%s_C"), *PathString, *AssetName));
	}

	template<typename WidgetType>
	void AddWidgetClassPath(const TSoftClassPtr<WidgetType>& WidgetClass, TArray<FSoftObjectPath>& OutPaths)
	{
		if (!WidgetClass.IsNull())
		{
			OutPaths.AddUnique(NormalizeWidgetClassPath(WidgetClass.ToSoftObjectPath()));
		}
	}
}

void UDBAUIFlowRegistry::GetWidgetClassPaths(TArray<FSoftObjectPath>& OutPaths) const
{
	OutPaths.Reset();
	AddWidgetClassPath(LoginWidgetClass, OutPaths);
	AddWidgetClassPath(CharacterSelectWidgetClass, OutPaths);
	AddWidgetClassPath(CharacterCreateWidgetClass, OutPaths);
	AddWidgetClassPath(SplashVideoWidgetClass, OutPaths);
	AddWidgetClassPath(MainLobbyWidgetClass, OutPaths);
	AddWidgetClassPath(LobbyPlayerHUDWidgetClass, OutPaths);
	AddWidgetClassPath(LobbyLoadingWidgetClass, OutPaths);
	AddWidgetClassPath(GameSettingsWidgetClass, OutPaths);
	AddWidgetClassPath(InventoryWidgetClass, OutPaths);
	AddWidgetClassPath(PartyPanelWidgetClass, OutPaths);
	AddWidgetClassPath(InvitePanelWidgetClass, OutPaths);
	AddWidgetClassPath(QueueModeSelectWidgetClass, OutPaths);
	AddWidgetClassPath(QueueStatusWidgetClass, OutPaths);
	AddWidgetClassPath(ReadyCheckWidgetClass, OutPaths);
	AddWidgetClassPath(MatchFoundWidgetClass, OutPaths);
	AddWidgetClassPath(PortalConfirmWidgetClass, OutPaths);
	AddWidgetClassPath(InteractionPromptWidgetClass, OutPaths);
	AddWidgetClassPath(NewbieVillageMainWidgetClass, OutPaths);
	AddWidgetClassPath(NewbieTaskTrackerWidgetClass, OutPaths);
	AddWidgetClassPath(ArenaHUDWidgetClass, OutPaths);
}
