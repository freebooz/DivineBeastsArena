/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Serilog;
using Serilog.Formatting.Compact;
using Game.Api.Middleware;
using Game.Api.Extensions;
using Game.Api.Endpoints.Auth;
using Game.Api.Endpoints.Admin;
using Game.Api.Endpoints.Account;
using Game.Api.Endpoints.Player;
using Game.Api.Endpoints.Config;
using Game.Api.Endpoints.Room;
using Game.Api.Endpoints.Match;
using Game.Api.Endpoints.Session;
using Game.Api.Endpoints.Runtime;
using Game.Api.Endpoints.Settlement;
using Game.Api.Endpoints.GameServer;
using Game.Api.Endpoints.Operation;
using Game.Api.Endpoints.Platform;
using Game.Api.Endpoints.Launcher;
using Game.Api.Endpoints.Feedback;
using Game.Api.Endpoints.Operations;
using Game.Infrastructure.Configuration;
using Game.Shared.Options;
using Game.Infrastructure.Database.Seed;
using Game.Infrastructure.Database;
using Microsoft.AspNetCore.Diagnostics.HealthChecks;
using Microsoft.EntityFrameworkCore;
using OpenTelemetry.Metrics;

Log.Logger = new LoggerConfiguration()
    .MinimumLevel.Information()
    .WriteTo.Console(new CompactJsonFormatter())
    .CreateLogger();

try
{
    var builder = WebApplication.CreateBuilder(args);
    builder.Configuration.AddSecretFilesFromEnvironment();

    builder.Host.UseSerilog();

    builder.Services.AddGameInfrastructure(builder.Configuration);
    builder.Services.AddGameOpenTelemetry(builder.Configuration);
    builder.Services.AddGameSwagger(builder.Configuration);
    builder.Services.AddGameHealthChecks(builder.Configuration);
    builder.Services.AddGameRateLimiting(builder.Configuration);
    builder.Services.AddSeedDataServices(builder.Configuration);

    builder.Services.AddSignalR();

    var app = builder.Build();

    if (app.Configuration.GetValue<bool>("Database:RunMigrationsAndExit"))
    {
        using var scope = app.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Log.Information("Running database migrations and exiting");
        await db.Database.MigrateAsync();
        Log.Information("Database migrations completed");
        return;
    }

    app.UseMiddleware<TraceIdMiddleware>();
    app.UseMiddleware<ExceptionHandlingMiddleware>();

    var env = app.Environment;
    var swaggerEnabled = builder.Configuration.GetValue<bool?>("Swagger:Enabled") ?? env.IsDevelopment();
    if (swaggerEnabled)
    {
        app.UseSwagger();
        app.UseSwaggerUI();
    }

    app.UseCors();
    app.UseRateLimiter();
    app.UseAuthentication();
    app.UseAuthorization();

    app.MapHealthChecks("/health/live", new HealthCheckOptions
    {
        Predicate = check => check.Tags.Contains("live")
    });
    app.MapHealthChecks("/health/ready", new HealthCheckOptions
    {
        Predicate = check => check.Tags.Contains("ready")
    });
    app.MapPrometheusScrapingEndpoint("/metrics");

    app.MapGet("/api/version", () => new { Version = "1.0.0", BuildTime = DateTimeOffset.UtcNow });

    app.MapHub<Games.Hubs.LobbyHub>("/hubs/lobby");

    // 注册所有 API 端点
    app.MapAuthEndpoints();
    app.MapAdminEndpoints();
    app.MapAccountEndpoints();
    app.MapPlayerEndpoints();
    app.MapConfigEndpoints();
    app.MapRoomEndpoints();
    app.MapMatchEndpoints();
    app.MapSessionEndpoints();
    app.MapRuntimeEndpoints();
    app.MapSettlementEndpoints();
    app.MapGameServerEndpoints();
    app.MapOperationEndpoints();
    app.MapPlatformEndpoints();
    app.MapLauncherEndpoints();
    app.MapFeedbackEndpoints();
    app.MapOperationsStatusEndpoints();

    await app.ApplySeedDataAsync();

    Log.Information("Game API starting on {Host}", app.Urls.FirstOrDefault() ?? "http://localhost:5000");
    app.Run();
}
catch (Exception ex)
{
    Log.Fatal(ex, "Application terminated unexpectedly");
}
finally
{
    Log.CloseAndFlush();
}
