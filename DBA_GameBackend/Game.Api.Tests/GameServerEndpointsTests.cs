/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 legacy /internal/servers 游戏服务器注册/查询接口的内部 API Key 保护。
- 阅读重点：/internal/game-servers 已有 Dedicated Server 编排保护；这里覆盖旧注册/查询接口。
- 修改提示：新增 GameServer 内部端点时，请同步覆盖缺失 key、错误 key 和正确 key 路径。
*/

using System.Net.Http.Json;
using System.Net;
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

public class GameServerEndpointsTests
{
    private const string InternalApiKey = "TEST-INTERNAL-API-KEY-MIN-32-CHARS";

    [Fact]
    public async Task ListManagedServers_WithoutInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();

        var (statusCode, response) = await GetApiResponseAsync(client, "/internal/game-servers/");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task ListManagedServers_WithWrongInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", "wrong-key");

        var (statusCode, response) = await GetApiResponseAsync(client, "/internal/game-servers/");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task ListManagedServers_WithInternalApiKey_ReachesManagerHandler()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);

        var response = await client.GetFromJsonAsync<ApiResponse<object>>("/internal/game-servers/");

        Assert.NotNull(response);
        Assert.True(response!.Success);
    }

    [Fact]
    public async Task GetLegacyInternalServer_WithoutInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetLegacyInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", "wrong-key");

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetLegacyInternalServer_WithInternalApiKey_ReachesGameServerHandler()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.NotFound, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("404|Not Found", response.Message);
    }

    private static async Task<(HttpStatusCode StatusCode, ApiResponse<object>? Response)> GetApiResponseAsync(
        HttpClient client,
        string path)
    {
        var httpResponse = await client.GetAsync(path);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();
        return (httpResponse.StatusCode, response);
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"game-server-endpoints-{Guid.NewGuid()}";
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
