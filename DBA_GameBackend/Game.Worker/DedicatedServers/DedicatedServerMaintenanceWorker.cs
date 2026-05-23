/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.Extensions.Options;
using Game.ServerManagement.DedicatedServers;

namespace Game.Worker.DedicatedServers;

public sealed class DedicatedServerMaintenanceWorker(
    IServiceScopeFactory scopeFactory,
    IOptions<DedicatedServerMaintenanceOptions> options,
    ILogger<DedicatedServerMaintenanceWorker> logger) : BackgroundService
{
    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var interval = TimeSpan.FromSeconds(Math.Max(5, options.Value.IntervalSeconds));
        logger.LogInformation("Dedicated server maintenance loop started, interval {IntervalSeconds}s", interval.TotalSeconds);

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                await using var scope = scopeFactory.CreateAsyncScope();
                var manager = scope.ServiceProvider.GetRequiredService<IDedicatedServerOrchestrator>();
                var affected = await manager.RunMaintenanceAsync(stoppingToken);
                if (affected > 0)
                {
                    logger.LogWarning("Dedicated server maintenance changed {Count} server(s)", affected);
                }
            }
            catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                logger.LogError(ex, "Dedicated server maintenance failed");
            }

            await Task.Delay(interval, stoppingToken);
        }
    }
}

public sealed class DedicatedServerMaintenanceOptions
{
    public const string Section = "DedicatedServerMaintenanceWorker";
    public int IntervalSeconds { get; init; } = 15;
}
