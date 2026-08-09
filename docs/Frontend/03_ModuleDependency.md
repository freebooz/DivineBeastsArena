# 03 UE 5.8 模块依赖审计与构建边界

## 本步骤变更

本步骤以本机 `D:/UnrealEngine-5.8.0-release` 的插件声明为准，调整了 `DivineBeastsArena` 应用模块、Game/Editor/Server Target 与 `.uproject`。没有新建业务模块、没有修改运行时业务代码、没有移动任何二进制资产。

| 项目 | 最终归属 | 理由 |
| --- | --- | --- |
| Core、CoreUObject、Engine | `DivineBeastsArena` 公共依赖 | 公共 UObject/Actor/Subsystem 类型的基础 |
| InputCore、EnhancedInput | `DivineBeastsArena` 公共依赖 | 现有 PlayerController 和公开输入契约使用 |
| HTTP、Json、JsonUtilities | `DivineBeastsArena` 私有依赖 | HTTP/JSON 实现在 Frontend/Account、Flow、Lobby 的私有 `.cpp` 中 |
| UMG、Slate、SlateCore | `DivineBeastsArena` 公共依赖 | 现有公共 Widget 头、`FSlateFontInfo` 和 UMG 基类暴露这些类型 |
| CommonUI、CommonInput、ModelViewViewModel | 非 Server 私有依赖 | 已启用的前台 UI/MVVM 基础；当前没有源码消费者，未来只能在 Frontend/UI 层使用 |
| Niagara | `DivineBeastsArena` 公共依赖 | 现有公开 Gameplay/VFX 类型使用 `UNiagaraSystem`/`UNiagaraComponent`；不属于纯前台 UI |
| GameBackendClient | `DivineBeastsArena` 公共依赖 | 现有公开 Frontend Facade 需要插件 DTO/委托类型 |
| GameCore、GameMoba | `DivineBeastsArena` 公共依赖 | 应用层复用通用 Subsystem/类型与 Gameplay 基础 |
| ReplicationGraph | `DivineBeastsArena` 公共依赖 | 现有复制图实现 |

## 插件核实

`.uproject` 已启用：`CommonUI`、`EnhancedInput`、`ModelViewViewModel`。本机 UE 5.8 的 `CommonUI.uplugin` 定义了 `CommonUI` 与 `CommonInput` 两个 Runtime 模块；`ModelViewViewModel.uplugin` 定义了 Runtime `ModelViewViewModel` 模块。因此不单独登记不存在的 `CommonInput` 插件。

## Target 边界

| Target | 定义 | UI 依赖策略 |
| --- | --- | --- |
| Game | `DBA_CLIENT=1`、`DBA_SERVER=0`、`DBA_WITH_FRONTEND_UI=1` | 可使用前台 UI/MVVM |
| Editor | 使用共享 UnrealEditor 构建环境，不添加自定义 GlobalDefinitions | 可通过非 Server ModuleRules 分支加载 CommonUI/MVVM 以进行人工资产审核 |
| Server | `DBA_CLIENT=0`、`DBA_SERVER=1`、`DBA_WITH_FRONTEND_UI=0` | 不加入 CommonUI、CommonInput、ModelViewViewModel、RenderCore、RHI、AudioMixer、MediaAssets 私有依赖 |

Editor Target 不能设置上述自定义 GlobalDefinitions：它与预编译 `UnrealEditor` 共享 BuildEnvironment，UBT 会拒绝不一致的全局定义。本步骤选择保持共享环境，避免为一个尚无消费者的宏启用 Unique BuildEnvironment。

## Dedicated Server 边界

`UDBAGameUIManager` 已有 `IsRunningDedicatedServer`/`NM_DedicatedServer` 运行时保护，Dedicated Server 不应创建 Widget。ModuleRules 现在进一步使 CommonUI、CommonInput 和 MVVM 不进入 Server 私有依赖列表。

UMG/Slate 目前仍无法从 Server 编译图移除：`DivineBeastsArena` 的公共 Widget 头和字体工具暴露 UMG/Slate 类型，`GameMoba` 也已有 UMG Widget 基类。Niagara 也仍在图中：共享 Gameplay/VFX 公共类型和实现使用 Niagara。直接删除这些依赖会破坏现有公共 API 或 Server 编译；后续应将纯前台 UI/Preview 与客户端表现 VFX 拆出独立非 Server 模块，再移除这些残留依赖。该拆分不属于本步骤的最小构建边界改动。

## 依赖方向

```text
DivineBeastsArena -> GameMoba -> GameCore
DivineBeastsArena -> GameCore
DivineBeastsArena -> GameBackendClient -> HTTP/Json
Frontend/UI (non-Server) -> CommonUI/CommonInput/ModelViewViewModel
```

禁止方向：`GameCore -> DivineBeastsArena`、`GameMoba -> Frontend/Online`、`GameBackendClient -> Widget/Gameplay`。Widget 不可直接依赖 HTTP、JSON、Token 或后端 DTO 解析；它只能经 Flow/Controller/ViewModel 传递意图。

## 工程检查

- Development Editor：构建成功。
- Win64 Development Server：构建成功，并生成 `DBA_GameClient/Binaries/Win64/DivineBeastsArenaServer.exe`。本机缺少既有 Server 产物，首次构建执行 1030 个引擎与项目动作，耗时约 12 分钟。
- UBT 仍报告既有 `VibeUE -> StructUtils`（UE 5.5 起弃用）警告；本步骤未调整该插件链。
