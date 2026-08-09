# 神兽竞技场 / Divine Beasts Arena

Unreal Engine 5.8 C++ 多人竞技项目。Unreal 工程名与主模块均为 `DivineBeastsArena`，项目代码前缀为 `DBA`。

## 开发基线

- 项目协作约束：[AGENTS.md](./AGENTS.md)
- 长期开发纲领：[ZodiacArena UE5.8 总控提示词](./docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md)
- 最终前台架构：[步骤29最终架构](./docs/Frontend/29_FinalArchitecture.md)
- 最终对象清单：[步骤29最终对象清单](./docs/Frontend/29_FinalObjectInventory.md)
- Deprecated 与删除记录：[步骤29迁移记录](./docs/Frontend/29_DeprecatedRemoved.md)

## 应用边界

| 目录 | 职责 |
| --- | --- |
| `DBA_GameClient/` | UE5.8 Windows/Android 客户端与 Linux Dedicated Server Target。 |
| `DBA_GameBackend/` | .NET 10 API、PostgreSQL、Redis、Worker 与 Dedicated Server 管理。 |
| `DBA_GameAdmin/` | Angular 运营管理后台。 |
| `DBA_GameWebsite/` | Next.js 官网。 |
| `DBA_GameLauncher/` | Tauri + React 启动器。 |
| `docs/` | 架构、开发、性能与人工审核记录。 |

## 前台生产主链

```text
Boot / Startup
  -> Login / Register / AutoLogin
  -> ServerSelect
  -> CharacterRosterLoading
  -> CharacterSelect
  -> Zodiac(+Appearance) -> Element -> FiveCamp -> Confirm/Name
  -> GameSession Enter Ticket
  -> FrontendTravelCoordinator
```

`EDBAFrontendState`、`UDBAFrontendFlowSubsystem`、`UDBAApiClientSubsystem`、`UDBACharacterRosterSubsystem`、`UDBACharacterCreateDraftSubsystem`、`UDBACharacterPreviewSubsystem` 分别是对应领域的唯一前台权威对象。Widget 只处理布局、动画和输入，不直接访问 HTTP、Token、数据库或执行 Travel。

`EDBAFiveCamp` 是 FiveCamp 的 canonical 运行时类型；`EDBAFiveCampType` 仅作为旧资产迁移类型。FiveCamp 不参与 `TeamId` 推导，生产主链禁止出现 Faction/FactionSelect。

## 验证边界

编译与静态检查属于工程检查。登录、选服、角色选择/创建、3D 预览、Android 触控、Dedicated Server 连接及完整 E2E 必须在可见运行环境中由人工审核，并单独记录结论。
