/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证客户端账号兼容接口必须使用 JWT 中的 player_id，而不能接受只有账号 subject 的 token。
- 阅读重点：测试签发真实 Bearer JWT，但故意省略 player_id / nameid claim，覆盖旧客户端角色接口的鉴权边界。
*/

using System.IdentityModel.Tokens.Jwt;
using System.Net;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Security.Claims;
using System.Text;
using Game.Infrastructure.Database;
using Game.Shared.Common;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using Microsoft.IdentityModel.Tokens;

namespace Game.Api.Tests;

public class AccountEndpointAuthorizationTests
{
    private const string JwtSecret = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-ACCOUNT-ENDPOINTS";
    private const string JwtIssuer = "GameApi.Tests";
    private const string JwtAudience = "GameApi.Tests";

    public static IEnumerable<object[]> CharacterEndpointsRequiringPlayerClaim()
    {
        yield return new object[] { HttpMethod.Get, "/api/account/characters", NoBody.Value };
        yield return new object[] { HttpMethod.Post, "/api/account/characters", new AccountCharacterRequest("Hero_Test", "Rat", "Water", (string?)null) };
        yield return new object[] { HttpMethod.Post, $"/api/account/characters/{Guid.NewGuid():N}/select", NoBody.Value };
        yield return new object[] { HttpMethod.Post, "/api/account/characters/select", new AccountSelectCharacterRequest(Guid.NewGuid().ToString("N")) };
        yield return new object[] { HttpMethod.Get, "/api/players/me/characters", NoBody.Value };
        yield return new object[] { HttpMethod.Post, "/api/players/me/characters", new AccountCharacterRequest("Hero_Test", "Rat", "Water", (string?)null) };
        yield return new object[] { HttpMethod.Post, $"/api/players/me/characters/{Guid.NewGuid():N}/select", NoBody.Value };
        yield return new object[] { HttpMethod.Post, "/api/players/me/characters/select", new AccountSelectCharacterRequest(Guid.NewGuid().ToString("N")) };
    }

    [Theory]
    [MemberData(nameof(CharacterEndpointsRequiringPlayerClaim))]
    public async Task CharacterEndpoints_WithJwtMissingPlayerId_ReturnUnauthorized(
        HttpMethod method,
        string path,
        object? body)
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue(
            "Bearer",
            CreateAccessTokenWithoutPlayerId());

        using var request = new HttpRequestMessage(method, path);
        if (!ReferenceEquals(body, NoBody.Value))
        {
            request.Content = JsonContent.Create(body);
        }

        var httpResponse = await client.SendAsync(request);
        var responseBody = await httpResponse.Content.ReadAsStringAsync();

        Assert.Equal(HttpStatusCode.Unauthorized, httpResponse.StatusCode);
        if (!string.IsNullOrWhiteSpace(responseBody))
        {
            var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();
            Assert.NotNull(response);
            Assert.False(response!.Success);
            Assert.Contains("401|Unauthorized", response.Message);
        }
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"account-endpoint-authorization-{Guid.NewGuid()}";
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
                        ["Jwt:Secret"] = JwtSecret,
                        ["Jwt:Issuer"] = JwtIssuer,
                        ["Jwt:Audience"] = JwtAudience,
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

    private static string CreateAccessTokenWithoutPlayerId()
    {
        var accountId = Guid.NewGuid();
        var claims = new[]
        {
            new Claim(JwtRegisteredClaimNames.Sub, accountId.ToString()),
            new Claim(ClaimTypes.Name, "AccountWithoutPlayer"),
            new Claim(JwtRegisteredClaimNames.Jti, Guid.NewGuid().ToString())
        };
        var key = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(JwtSecret));
        var creds = new SigningCredentials(key, SecurityAlgorithms.HmacSha256);
        var token = new JwtSecurityToken(
            issuer: JwtIssuer,
            audience: JwtAudience,
            claims: claims,
            expires: DateTime.UtcNow.AddMinutes(30),
            signingCredentials: creds);

        return new JwtSecurityTokenHandler().WriteToken(token);
    }

    private sealed record AccountCharacterRequest(
        string? CharacterName,
        string? Zodiac,
        string? PrimaryElement,
        string? FiveCamp);

    private sealed record AccountSelectCharacterRequest(string? CharacterId);

    private sealed class NoBody
    {
        public static readonly NoBody Value = new();

        private NoBody()
        {
        }
    }
}
