# 步骤29：Deprecated、删除与保留记录

## 删除结果

本步骤删除的 Legacy 运行时对象：**无**。

这不是遗漏，而是严格执行删除门槛后的结果：当前没有对象同时满足“源码/Config/软引用/资产无引用、替代链已经落地、人工 E2E 已覆盖”三项条件。步骤27只完成静态/工程检查，未记录人工业务验收通过；当前也未完成 Editor Reference Viewer 与 Redirector 审核。

## 本次正式 Deprecated

| 对象/API | Deprecated 方式 | Successor | 新调用规则 |
| --- | --- | --- | --- |
| `EDBALoginFlowState` | UE5.8 UENUM `Deprecated` metadata | `EDBAFrontendState` | 禁止新增枚举值、Guard 或页面依赖 |
| `EDBAFrontendStep` | UE5.8 UENUM `Deprecated` metadata | `EDBAFrontendState` | 仅存量反序列化 |
| `EDBAZodiacType` | UE5.8 UENUM `Deprecated` metadata | `EDBAZodiac` | 只经 Adapter 迁移 |
| `EDBAElementType` | UE5.8 UENUM `Deprecated` metadata | `EDBAElement` | 只经 Adapter 迁移 |
| `EDBAFiveCampType` | UE5.8 UENUM `Deprecated` metadata | `EDBAFiveCamp` | 只经 Adapter 迁移 |
| `UDBAFrontendFlowController` | `DeprecatedNode` metadata | 页面 WidgetController + Flow | 禁止创建新实例 |
| `UDBAGameUIManager` 登录 Flow 显示 API | `DeprecatedFunction` metadata | UILayerManager + Flow | 仅现有 GameInstance/Startup/Splash 兼容调用 |
| `/api/auth` 中已有 v1 successor 的路由 | `Deprecation` + successor `Link` 响应头 | `/api/v1/auth/*` | 新客户端禁止使用 |
| `/api/account/characters` | `Deprecation` + successor `Link` 响应头 | `/api/v1/characters` | 新客户端禁止使用 |
| `/api/players/me/characters` | `Deprecation` + successor `Link` 响应头 | `/api/v1/characters` | 新客户端禁止使用 |
| `/api/v1/auth/player-name/ensure` | `Deprecation` + successor `Link` 响应头 | `/api/v1/auth/player-name/generate` | 新客户端只调用 `/generate` |

`UDBAZodiacDataAsset`、`DeprecatedLegacyClassificationId` 和旧 Village 私有链此前已经带有 Deprecated/迁移注释，本步骤继续保留。

## FiveCamp 收口

最终 canonical 类型选择 `EDBAFiveCamp`。旧 `EDBAFiveCampType` 只能经 `DBAIdentityTypeAdapter` 转换，不能出现在新 ViewModel、Draft、Roster、Preview 或后端 v1 DTO 中。

未发现 `DivinePantheon` 运行时类型或存量资产路径需要建立额外 Mapper；因此没有创建第二套 Pantheon Adapter。`Faction` 仅出现在禁止性文档、迁移哨兵字段和校验错误文本中，不属于运行时主流程，也没有新增 Faction GameplayTag。

## 暂不能删除

| 对象 | 阻塞证据 | 删除前置条件 |
| --- | --- | --- |
| Lobby/Frontend 两套同名 Character WBP | Lobby 资产被 Registry 指向；Frontend 资产引用未知 | Editor Reference Viewer、父类 Compile、人工 E2E、迁移后 Fix Redirectors |
| `UDBAFrontendFlowController` | 旧 Login/Character FlowWidget 与 UIManager 源码直接调用 | 旧 WBP 重父类到页面专属 Controller 后再删除 |
| `UDBAGameUIManager` | 仍负责大厅、HUD、设置、背包、组队和旧前台挂载 | 只移除前台兼容分支，Manager 本体不能按本任务删除 |
| `ADBACharacterPresentationActor` | GameMode/旧 Widget/Preview fallback 仍引用 | 地图与旧资产全部迁至 PreviewStage 后删除 fallback |
| 旧 Account Character API/Application UseCases | 已发布插件仍调用 `/api/players/me/characters` | 插件升级、发布窗口结束、调用指标归零 |
| 旧 Village Flow 私有字段/回调 | 大厅兼容代码仍存在 | GameSession/Travel 人工联调覆盖返回大厅后删除 |

## Redirector 处理结果

未执行 Fix Redirectors。该操作会修改二进制资产，必须在 UE5.8 Editor 中以可见事务完成 Compile、Save 和人工引用审核；当前没有证据支持安全资产迁移。文本扫描没有发现可可靠判定的 Redirector，但文件名/二进制字符串扫描不能替代 Asset Registry。

## 回滚性

本步骤只增加 Deprecated metadata、兼容 API 响应头和文档，不改变旧接口的请求/响应主体，不删除资产，也不改动地图。若旧客户端出现兼容问题，可独立回退响应头/metadata，不影响 v1 权威业务和数据库数据。
