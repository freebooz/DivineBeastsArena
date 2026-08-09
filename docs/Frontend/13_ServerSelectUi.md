# 步骤13：UE 服务器选择界面

## 审计与收敛

| 现有对象 | 结论 | 本步骤处理 |
| --- | --- | --- |
| `UDBAServerDirectorySubsystem` | KEEP | 保持为唯一玩家区服目录入口；补充重复刷新保护与上次登录区服偏好。 |
| `UDBAFrontendFlowSubsystem` | ADAPT | `SelectServer` 仍是唯一写入 `FrontendSessionContext.ServerId` 的入口；确认后转换至 `CharacterRosterLoading`。 |
| `UDBAOnlineAccountService::GetCharacterList` | KEEP | 继续承担现有角色列表读取；不在选服 Widget 内重建第二套 Roster 请求。 |
| `UDBAGameUIManager` | DEPRECATE-LATER | 仍为旧 Login/角色页面兼容门面；新选服 Screen 由 `UDBAUILayerManagerSubsystem` 的 `ScreenLayer` 激活。 |
| `GameServerInstance` | KEEP（内部） | Dedicated Server 实例登记，与玩家区服目录无关，不进入选服 UI。 |

仓库中不存在 `WBP_DBA_ServerSelect` 或 `WBP_DBA_ServerListItem` 二进制资产，也没有可复用的同义选服页面。因仓库规则禁止直接编辑 `.uasset`，本步骤提供 C++ 父类和轻量原生回退布局；后续可在 Editor 创建同名 Widget Blueprint 并仅配置布局、动画、SafeZone 与资源引用。

## 唯一链路

```text
UDBAServerDirectorySubsystem
  -> UDBAServerSelectWidgetController
  -> UDBAServerSelectViewModel
  -> UDBAServerSelectScreenBase / UDBAServerListItemWidgetBase
  -> UDBAUILayerManagerSubsystem.ScreenLayer

用户确认 ServerId
  -> UDBAFrontendFlowSubsystem::SelectServer
  -> FrontendSessionContext.ServerId
  -> CharacterRosterLoading
  -> UDBAOnlineAccountService::GetCharacterList
```

- Widget 只显示 ViewModel、响应按钮与触控列表事件；不访问 HTTP、Token、数据库或原始 JSON。
- Controller 只转发刷新、重试、选择、确认与返回意图。
- Flow 再次校验 ServerId 是否来自可选择目录项，随后写入 SessionContext 并驱动角色列表状态。
- 维护、离线与已满区服的条目保留展示，但按钮禁用并显示维护文案或不可进入原因。
- `LastServerId.<AccountId>` 只写入 GameUserSettings 配置，内容为稳定区服 ID；不保存连接地址、Password、AccessToken、RefreshToken 或 GameTicket。

## 资源与人工绑定

在 Unreal Editor 中创建并保存以下资产后，将其软类写入 `DBAFrontendSettings.ServerSelectScreenWidgetClass`：

- `Content/DBA/UI/Frontend/Server/WBP_DBA_ServerSelect`：父类 `UDBAServerSelectScreenBase`。
- `Content/DBA/UI/Frontend/Server/WBP_DBA_ServerListItem`：父类 `UDBAServerListItemWidgetBase`，并在前者设置 `ServerListItemWidgetClass`。

页面应在 SafeZone 内使用可滚动、足够触控高度的列表。可选绑定控件名为 `TitleText`、`StatusText`、`ServerList`、`RefreshButton`、`RetryButton`、`ConfirmButton`、`SelectButton`、`SummaryText`。不需要 Blueprint 业务图逻辑。

## 人工审核步骤

1. 使用已登录账号进入 `ServerSelect`，确认 ScreenLayer 只展示一个选服页面。
2. 验证名称、区域、状态、人数、推荐和“上次登录”标记可见。
3. 验证维护、离线和满员项不可点击，维护原因可见。
4. 选择 Online 或 Busy 区服并确认，观察状态依次到 `CharacterRosterLoading`、角色选择或角色创建。
5. 断开目录服务后执行刷新，确认旧列表仍可显示、错误可重试且不黑屏。
