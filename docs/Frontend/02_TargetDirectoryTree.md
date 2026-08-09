# 02 目标目录树与模块边界

## Source 目标目录

```text
DBA_GameClient/Source/DivineBeastsArena/
  Public/GameDBA/
    Core/
      GameInstance/
      Framework/
    Frontend/
      Flow/
      Session/
      UI/
        Layers/
        Screens/
          Startup/
          Auth/
          ServerDirectory/
          Character/
        Controllers/
        ViewModels/
        Data/
      Preview/
    Online/
      Http/
      Auth/
        SecureStorage/
      ServerDirectory/
      Character/
        Contracts/
        Roster/
        Create/
      Session/
    Character/
      Data/
        Zodiac/
      Appearance/
    Gameplay/
  Private/GameDBA/
    (与 Public 同构)
```

目录是目标归属，而不是批量移动命令。当前 `Framework`、`UI`、`Data`、`Frontend`、`Characters`、`Gameplay`、`Presentation` 继续保持原位并按 02_OwnershipMatrix 的 Owner 逐个迁移。`GameCore` 保持通用基础模块；`GameMoba` 保持 Gameplay/GAS 基础模块；两者禁止依赖应用模块。

## Content 目标目录

```text
DBA_GameClient/Content/DBA/
  Maps/
    Frontend/
      L_DBA_Boot
      L_DBA_Frontend
  UI/
    Frontend/
      Common/
      Startup/
      Auth/
      ServerDirectory/
      Character/
  Characters/
    Zodiac/
    Preview/
  Data/
    Zodiac/
      FiveCamp/
      Appearance/
    UI/
      Frontend/
  VFX/
    Frontend/
  Audio/
    Frontend/
```

## 依赖规则

| 层 | 可依赖 | 不可依赖 |
| --- | --- | --- |
| `GameCore` | Engine、Core、通用类型/基础 Subsystem | `DivineBeastsArena`、前台 UI、应用后端实现 |
| `GameMoba` | GameCore、GAS、Gameplay 相关引擎模块 | Frontend、Online、Preview |
| `GameBackendClient` | HTTP/JSON、DeveloperSettings | 应用 Widget、Gameplay、GameCore 的应用实现 |
| `DivineBeastsArena/Core` | GameCore、GameMoba、插件契约 | UI 到后端的越层访问 |
| `Frontend/UI` | Flow 的只读状态、Controller/ViewModel、软类资产 | HTTP、Token、数据库、Travel |
| `Frontend/Preview` | Zodiac/Appearance 数据、软表现资源 | GAS、Combat、Replication、Dedicated Server |
| `Online/*` | DTO、ApiClient、Session/Character/Auth 边界 | UMG、Widget、Preview Actor |

## Dedicated Server 收敛约束

当前 `DivineBeastsArena.Build.cs` 对 UMG、Niagara 有模块依赖，Server Target 仍编译应用模块。这不能满足最终“Dedicated Server 依赖图不存在纯前台 UMG/Niagara 强依赖”的目标。后续分两步处理：先让 Frontend/UI 与 Frontend/Preview 仅在非 Server 目标创建和加载；再依据实际头文件依赖，将纯前台代码隔离到不会由 Server Target 构建的模块或条件依赖。不得在本步骤为了目录目标直接移除现有依赖。

## 资产迁移规则

1. 每次只迁移一个已在 Reference Viewer 审核过的资产族。
2. 移动后在 Unreal Editor 执行 Fix Redirectors、Compile、Save，并由人工核对引用地图和 Widget。
3. 未确认的 Frontend/Lobby 重复 WBP、生肖模型双域与 DataAsset 继续保留。
4. 不直接编辑 `.uasset`、`.umap` 二进制内容。

