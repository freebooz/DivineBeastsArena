/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 Admin 列表接口缺省分页参数时仍能返回第一页，避免运维直接访问列表 URL 时触发 500。
- 阅读重点：测试使用真实 WebApplicationFactory、JWT 登录和 InMemory 数据库，不依赖外部服务。
*/

using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace Game.Api.Tests;

public class AdminListPagingDefaultsTests
{
    private const string AdminPassword = "Admin@123456";

    public static IEnumerable<object[]> ListEndpointsWithoutPaging()
    {
        yield return new object[] { "/api/admin/players" };
        yield return new object[] { "/api/admin/audit-logs" };
        yield return new object[] { "/api/admin/feedback" };
        yield return new object[] { "/api/admin/support/tickets" };
        yield return new object[] { "/api/admin/matches" };
        yield return new object[] { "/api/admin/servers" };
        yield return new object[] { "/api/admin/client-versions" };
    }

    public static IEnumerable<object[]> PaginatedEndpointsWithoutPaging()
    {
        yield return new object[] { "/api/admin/inventory/logs" };
        yield return new object[] { "/api/feedback/recent" };
        yield return new object[] { "/api/rankings/ranked" };
    }

    [Theory]
    [MemberData(nameof(ListEndpointsWithoutPaging))]
    public async Task AdminListEndpoints_WithoutPagingQuery_ReturnDefaultPage(string path)
    {
        await using var factory = CreateFactory();
        await SeedAdminAsync(factory);

        var client = factory.CreateClient();
        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var httpResponse = await client.GetAsync(path);
        var responseBody = await httpResponse.Content.ReadAsStringAsync();

        Assert.True(httpResponse.IsSuccessStatusCode, responseBody);

        using var payload = JsonDocument.Parse(responseBody);
        var root = payload.RootElement;
        Assert.True(root.GetProperty("success").GetBoolean());

        var data = root.GetProperty("data");
        Assert.Equal(1, data.GetProperty("page").GetInt32());
        Assert.Equal(50, data.GetProperty("pageSize").GetInt32());
    }

    [Theory]
    [MemberData(nameof(PaginatedEndpointsWithoutPaging))]
    public async Task PaginatedEndpoints_WithoutPagingQuery_DoNotReturnServerError(string path)
    {
        await using var factory = CreateFactory();
        await SeedAdminAsync(factory);

        var client = factory.CreateClient();
        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var httpResponse = await client.GetAsync(path);
        var responseBody = await httpResponse.Content.ReadAsStringAsync();

        Assert.True(httpResponse.IsSuccessStatusCode, responseBody);

        using var payload = JsonDocument.Parse(responseBody);
        Assert.True(payload.RootElement.GetProperty("success").GetBoolean());
    }

    private static async Task<string> LoginAsync(HttpClient client)
    {
        var login = await client.PostAsJsonAsync(
            "/api/admin/auth/login",
            new AdminLoginRequest("ops-admin", AdminPassword));
        login.EnsureSuccessStatusCode();

        var payload = await login.Content.ReadFromJsonAsync<ApiResponse<AdminLoginResponse>>();
        Assert.NotNull(payload);
        Assert.True(payload!.Success);
        Assert.NotNull(payload.Data);
        return payload.Data!.AccessToken;
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"admin-list-paging-defaults-{Guid.NewGuid()}";
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
                        ["Jwt:Secret"] = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-ADMIN-LIST-ENDPOINTS",
                        ["Jwt:Issuer"] = "GameApi.Tests",
                        ["Jwt:Audience"] = "GameApi.Tests",
                        ["InternalApi:Key"] = "TEST-INTERNAL-API-KEY-MIN-32-CHARS",
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

    private static async Task SeedAdminAsync(WebApplicationFactory<Program> factory)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var now = DateTimeOffset.UtcNow;

        db.AdminUsers.Add(new AdminUser
        {
            Id = Guid.NewGuid(),
            Username = "ops-admin",
            PasswordHash = BCrypt.Net.BCrypt.HashPassword(AdminPassword),
            Role = "OPS",
            Status = "ACTIVE",
            CreatedAt = now
        });

        await db.SaveChangesAsync();
    }
}
