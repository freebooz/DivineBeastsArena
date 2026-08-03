/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：为 EF Core 设计时命令创建 DbContext，并复用运行时数据库环境变量。
- 安全约束：禁止硬编码数据库地址、用户名或密码；缺少连接配置时立即中文报错。
*/

using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Design;
namespace Game.Infrastructure.Database;

public class GameDbContextFactory : IDesignTimeDbContextFactory<GameDbContext>
{
    public GameDbContext CreateDbContext(string[] args)
    {
        var connectionString = Environment.GetEnvironmentVariable("Database__ConnectionString");
        if (string.IsNullOrWhiteSpace(connectionString))
        {
            connectionString = Environment.GetEnvironmentVariable("DATABASE_CONNECTION_STRING");
        }
        if (string.IsNullOrWhiteSpace(connectionString))
        {
            throw new InvalidOperationException(
                "EF 设计时操作缺少数据库连接配置，请设置 Database__ConnectionString 或 DATABASE_CONNECTION_STRING。");
        }

        var optionsBuilder = new DbContextOptionsBuilder<GameDbContext>();
        optionsBuilder.UseNpgsql(connectionString);

        return new GameDbContext(optionsBuilder.Options);
    }
}
