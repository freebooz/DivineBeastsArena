/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;

namespace Game.Worker;

public class Worker(
    IServiceScopeFactory scopeFactory,
    IOptions<WorkerJobOptions> options,
    ILogger<Worker> logger) : BackgroundService
{
    private static readonly string[] ActiveServerStatuses = { "STARTING", "READY", "IDLE", "ALLOCATED", "RUNNING" };

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var interval = TimeSpan.FromSeconds(Math.Max(5, options.Value.IntervalSeconds));
        logger.LogInformation("Game.Worker 已启动，后台任务间隔 {IntervalSeconds} 秒", interval.TotalSeconds);

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                await RunMaintenanceCycleAsync(stoppingToken);
            }
            catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                logger.LogError(ex, "后台维护任务执行失败，将在下一个周期重试");
            }

            await Task.Delay(interval, stoppingToken);
        }
    }

    private async Task RunMaintenanceCycleAsync(CancellationToken cancellationToken)
    {
        await using var scope = scopeFactory.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();

        var staleCount = await MarkStaleServersAsync(db, cancellationToken);
        await EnsureDailyStatsRowAsync(db, cancellationToken);

        logger.LogInformation("后台维护任务完成：过期服务端 {StaleCount} 个", staleCount);
    }

    private async Task<int> MarkStaleServersAsync(GameDbContext db, CancellationToken cancellationToken)
    {
        var staleCutoff = DateTimeOffset.UtcNow.AddSeconds(-Math.Max(30, options.Value.ServerHeartbeatTimeoutSeconds));

        var staleServers = await db.GameServerInstances
            .Where(x =>
                ActiveServerStatuses.Contains(x.Status) &&
                x.LastHeartbeatAt != null &&
                x.LastHeartbeatAt < staleCutoff)
            .ToListAsync(cancellationToken);

        foreach (var server in staleServers)
        {
            var previousStatus = server.Status;
            server.Status = "STALE";
            server.EndedAt ??= DateTimeOffset.UtcNow;
            server.UpdatedAt = DateTimeOffset.UtcNow;
            server.CrashReason ??= "后台任务检测到服务端心跳超时。";

            db.GameServerEvents.Add(new GameServerEvent
            {
                ServerId = server.Id,
                EventType = "STALE_DETECTED",
                PayloadJson = $$"""{"previousStatus":"{{previousStatus}}","lastHeartbeatAt":"{{server.LastHeartbeatAt:O}}"}""",
                CreatedAt = DateTimeOffset.UtcNow
            });

            if (server.Port > 0)
            {
                var allocation = await db.PortAllocations.FirstOrDefaultAsync(x => x.Port == server.Port, cancellationToken);
                if (allocation is not null && allocation.ServerId == server.Id)
                {
                    allocation.Status = "FREE";
                    allocation.ServerId = null;
                    allocation.ReleasedAt = DateTimeOffset.UtcNow;
                }
            }
        }

        if (staleServers.Count > 0)
        {
            await db.SaveChangesAsync(cancellationToken);
        }

        return staleServers.Count;
    }

    private static async Task EnsureDailyStatsRowAsync(GameDbContext db, CancellationToken cancellationToken)
    {
        var today = DateTimeOffset.UtcNow.Date;
        var hasToday = await db.DailyStats.AnyAsync(x => x.Date == today && x.Region == "global", cancellationToken);
        if (hasToday)
        {
            return;
        }

        db.DailyStats.Add(new DailyStats
        {
            Date = today,
            Region = "global",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await db.SaveChangesAsync(cancellationToken);
    }
}
