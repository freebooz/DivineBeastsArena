/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Worker;
using Game.Infrastructure.Configuration;
using Game.Infrastructure.Database;
using Game.ServerManagement.DedicatedServers;
using Game.Shared.Options;
using Game.Worker.DedicatedServers;
using Microsoft.EntityFrameworkCore;
using Serilog;
using Serilog.Formatting.Compact;

var builder = Host.CreateApplicationBuilder(args);
builder.Configuration.AddSecretFilesFromEnvironment();

Log.Logger = new LoggerConfiguration()
    .MinimumLevel.Information()
    .WriteTo.Console(new CompactJsonFormatter())
    .CreateLogger();

builder.Services.AddSerilog();

var databaseOptions = builder.Configuration.GetSection(DatabaseOptions.Section).Get<DatabaseOptions>() ?? new();
var dedicatedServerOptions = builder.Configuration.GetSection(DedicatedServerOrchestrationOptions.Section).Get<DedicatedServerOrchestrationOptions>() ?? new();
RequiredOptionsValidator.ValidateDatabase(databaseOptions);
RequiredOptionsValidator.ValidateDedicatedServerOrchestration(dedicatedServerOptions);

builder.Services.AddDbContext<GameDbContext>(options => options.UseNpgsql(databaseOptions.ConnectionString));
builder.Services.Configure<MaintenanceWorkerOptions>(builder.Configuration.GetSection(MaintenanceWorkerOptions.Section));
builder.Services.Configure<DedicatedServerOrchestrationOptions>(builder.Configuration.GetSection(DedicatedServerOrchestrationOptions.Section));
builder.Services.Configure<DedicatedServerMaintenanceOptions>(builder.Configuration.GetSection(DedicatedServerMaintenanceOptions.Section));
builder.Services.AddScoped<IDedicatedServerOrchestrator, DedicatedServerOrchestrator>();
builder.Services.AddHostedService<MaintenanceWorker>();
builder.Services.AddHostedService<DedicatedServerMaintenanceWorker>();

var host = builder.Build();
host.Run();

