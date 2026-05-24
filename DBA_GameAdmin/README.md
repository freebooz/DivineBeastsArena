# GameAdmin

Angular 18+ 管理后台。当前工程使用 Angular 21，满足 18+ 技术栈要求，并以 SPA 方式调用 `Game.Api` 的 `/api/admin/*`、`/api/live-ops/status` 和 `/api/platform/applications`。

## 构建

```bash
npm ci
npm run build
```

## 本地运行

```bash
npm start
```

默认生产构建使用同源 API。开发时可通过反向代理或本地网关把 `/api` 指向 `Game.Api`。

## 功能页面

- Dashboard：生产运营指标和健康状态
- 玩家与玩家详情
- 对局与对局详情
- Dedicated Server 管理和 Kill 高危操作
- 游戏配置与客户端版本管理
- 玩家反馈、客服工单、审计日志
- 平台应用结构与运行命令

旧 Blazor Server 版本已归档到 `DBA_GameAdmin_LegacyBlazor`，用于迁移比对和回溯。
