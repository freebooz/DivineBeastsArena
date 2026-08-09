# 步骤25：角色选择与创建恢复矩阵

## 现有实现 → 目标实现 → 迁移方法

| 现有实现 | 状态 | 目标实现 | 迁移方法 |
| --- | --- | --- | --- |
| `UDBACharacterRosterSubsystem::DeleteCharacter` | KEEP / EXTEND | 服务端 SoftDelete 后的缓存、选中态回收 | 删除成功后由控制器释放旧预览；Roster 刷新时统一回收选中角色。 |
| `UDBACharacterRosterSubsystem::RefreshCharacterList` | KEEP / EXTEND | AccountId + ServerId + RequestGeneration 作用域缓存 | 刷新保留仍存在的 `CharacterId`，不存在则服务端选中项、首项或空态。 |
| `UDBAFrontendFlowSubsystem::BeginServerSelection` | KEEP / EXTEND | 唯一换服清理入口 | 先失效旧回调，再清 Roster、Preview、Draft、选中角色和 ServerId。 |
| `UDBAFrontendFlowSubsystem::RequestLogout` | KEEP / EXTEND | 唯一登出清理入口 | `AccountService` 清 Token；Flow 清账号以外的前台业务上下文与预览。 |
| `UDBAApiClientSubsystem` 401 单飞刷新 | KEEP / EXTEND | Refresh 失败通知唯一 Flow | 新增不含敏感数据的失败事件；Flow 统一登出，页面不各自清会话。 |
| Android 生命周期 | ADD | 后台失效异步代次、前台检查会话 | 使用 UE `FCoreDelegates`，不创建第二个 UI Root。 |

## 恢复策略

| 场景 | 清理与回退 | 重试语义 |
| --- | --- | --- |
| 删除角色 | Modal 确认 → 服务端 SoftDelete → 移除缓存 → 释放旧 Preview → 选择下一项或空态 | 失败保留当前角色与 Modal 外的列表；不清其他缓存。 |
| 刷新角色列表 | `RequestGeneration` 校验 AccountId、ServerId；保留有效选中 ID | 原 ID 不存在时自动选择服务端标记角色、首项或清空。 |
| 换服 | 清 Roster、SelectedCharacter、Preview、CreateDraft、旧 ServerId | 仅在新 `ServerId` 写入会话后加载角色列表。 |
| 登出 | 取消旧异步、清 Roster/Preview/Draft/Server/账号摘要；AccountService 清 Token | 回 Login，不保留旧账号角色或预览。 |
| 401 / Token 过期 | ApiClient 单飞 Refresh；失败只通知 Flow 一次 | Flow 统一调用登出，不允许页面各自处理。 |
| 网络中断 | 广播结构化错误并短暂进入 `RecoverableError` 后回可重试 Screen | NetworkStatus/现有 Retry 入口消费事件；不创建无限 Modal。 |
| 目录维护 | 清角色作用域，回 `ServerSelect` 并刷新目录 | 维护解除后用户可选服重试。 |
| 全局维护 | 清角色作用域，回 `Startup` | 启动页可重新检查服务状态。 |
| Android 后台/恢复 | 后台失效旧回调；恢复后刷新 Token；按原 Screen 刷新目录或角色列表 | 创建草稿页面仅验证会话，不重建 UI Root 或重置 Draft。 |

## 依赖边界

```text
ApiClient(401 单飞刷新失败)
  -> FrontendFlow(唯一登出/恢复决策)
  -> Roster(缓存与请求取消) + Preview(资源释放) + Draft(临时数据清除)
  -> Controller/ViewModel(事件驱动更新)
```

`Widget` 不直接处理 Token、HTTP、缓存或 Preview 生命周期。所有异步回调继续受 ApiClient 的 SessionGeneration、Roster 的 AccountId/ServerId/RequestGeneration 与 Flow 的 FlowRequestGeneration 共同约束。

## 验证说明

新增状态机契约覆盖从 `RecoverableError` 回确认创建步骤与启动页的合法路径。根据仓库人工审核策略，不自动执行业务测试；需要在可见客户端由人工验证删除、换服、登出、断网恢复和 Android 前后台流程。
