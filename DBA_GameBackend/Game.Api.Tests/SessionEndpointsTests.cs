/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 /internal/sessions 管理接口的内部 API Key 保护。
- 阅读重点：这里只覆盖内部管理面；/api/sessions 客户端接口仍按 JWT/玩家会话规则处理。
- 修改提示：新增 Session 内部管理端点时，请同步覆盖缺失 key、错误 key 和正确 key 路径。
*/

using System.Net.Http.Json;
using Game.Infrastructure.Database;
using Game.Shared.Common;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace Game.Api.Tests;

public class SessionEndpointsTests
{
    private const string InternalApiKey = "TEST-INTERNAL-API-KEY-MIN-32-CHARS";

    [Fact]
    public async Task MarkInProgress_WithoutInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();

        var response = await client.PostAsJsonAsync($"/internal/sessions/{Guid.NewGuid()}/mark-in-progress", new { });
        var body = await response.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.NotNull(body);
        Assert.False(body!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", body.Message);
    }

    [Fact]
    public async Task MarkInProgress_WithWrongInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", "wrong-key");

        var response = await client.PostAsJsonAsync($"/internal/sessions/{Guid.NewGuid()}/mark-in-progress", new { });
        var body = await response.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.NotNull(body);
        Assert.False(body!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", body.Message);
    }

    [Fact]
    public async Task MarkInProgress_WithInternalApiKey_ReachesSessionHandler()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);

        var response = await client.PostAsJsonAsync($"/internal/sessions/{Guid.NewGuid()}/mark-in-progress", new { });
        var body = await response.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.NotNull(body);
        Assert.False(body!.Success);
        Assert.Contains("404|Not Found", body.Message);
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"session-endpoints-{Guid.NewGuid()}";
        return new WebApplicationFactory<Program>()
            .WithWebHostBuilder(builder =>
            {
                builder.UseEnvironment("Development");
                builder.ConfigureAppConfiguration((_, config) =>
                {
                    config.AddInMemoryCollection(new Dictionary<string, string?>
                    {
                        ["Database:ConnectionString"] = "Host=localhost;Database=dba_test;Username=test;Password=test",
                        ["Redis:ConnectionString"] = "localhost:6379",
                        ["Jwt:Secret"] = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-ENDPOINTS",
                        ["Jwt:Issuer"] = "GameApi.Tests",
                        ["Jwt:Audience"] = "GameApi.Tests",
                        ["InternalApi:Key"] = InternalApiKey,
                        ["GameServerManager:ServerMode"] = "External",
                        ["GameServerManager:PublicIp"] = "127.0.0.1",
                        ["GameServerManager:BackendUrl"] = "http://localhost:8080",
                        ["SeedData:Enabled"] = "false",
                        ["Swagger:Enabled"] = "false"
                    });
                });
                builder.ConfigureServices(services =>
                {
                    services.RemoveAll<DbContextOptions<GameDbContext>>();
                    services.RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>();
                    services.AddDbContext<GameDbContext>(options => options.UseInMemoryDatabase(dbName));
                });
            });
    }
}
