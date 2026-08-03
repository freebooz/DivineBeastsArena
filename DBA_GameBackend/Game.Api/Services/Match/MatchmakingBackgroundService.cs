/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：周期性推进匹配队列配对与超时清理，避免仅依赖客户端轮询。
*/

using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace Game.Api.Services.Match;

public sealed class MatchmakingBackgroundService(
    IServiceScopeFactory scopeFactory,
    ILogger<MatchmakingBackgroundService> logger) : BackgroundService
{
    private static readonly TimeSpan Interval = TimeSpan.FromSeconds(2);

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        logger.LogInformation("匹配后台服务已启动，间隔 {IntervalSeconds} 秒", Interval.TotalSeconds);

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                await using var scope = scopeFactory.CreateAsyncScope();
                var matchService = scope.ServiceProvider.GetRequiredService<IMatchService>();
                await matchService.ProcessMatchmakingTickAsync(stoppingToken);
            }
            catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                logger.LogError(ex, "匹配后台周期执行失败，将在下一周期重试");
            }

            try
            {
                await Task.Delay(Interval, stoppingToken);
            }
            catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested)
            {
                break;
            }
        }
    }
}
