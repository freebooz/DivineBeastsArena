/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Seed;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace Game.Api.Extensions;

public static class SeedDataApplicationExtensions
{
    public static async Task ApplySeedDataAsync(this WebApplication app)
    {
        var logger = app.Services.GetRequiredService<ILogger<DevelopmentDataSeeder>>();
        var seedDataOptions = app.Services.GetRequiredService<SeedDataOptions>();

        if (!seedDataOptions.Enabled)
        {
            logger.LogInformation("SeedData is disabled, skipping seed operation");
            return;
        }

        var env = app.Environment;
        if (env.IsProduction())
        {
            logger.LogWarning("SeedData is configured but will NOT run in Production environment");
            return;
        }

        using var scope = app.Services.CreateScope();
        var context = scope.ServiceProvider.GetRequiredService<GameDbContext>();

        if (!context.Database.IsRelational())
        {
            logger.LogWarning("Database provider does not support seeding (not relational)");
            return;
        }

        logger.LogInformation("正在迁移数据库结构...");
        await context.Database.MigrateAsync();
        logger.LogInformation("数据库结构迁移完成");

        var seeder = scope.ServiceProvider.GetRequiredService<DevelopmentDataSeeder>();
        await seeder.SeedAsync();

        logger.LogInformation("SeedData applied successfully");
    }
}
