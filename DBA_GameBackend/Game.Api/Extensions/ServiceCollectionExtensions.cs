/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.EntityFrameworkCore;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.IdentityModel.Tokens;
using Microsoft.OpenApi.Models;
using OpenTelemetry.Metrics;
using OpenTelemetry.Resources;
using OpenTelemetry.Trace;
using Game.Infrastructure.Database;
using Game.Infrastructure.Redis;
using Game.Infrastructure.Auth;
using Game.Shared.Options;
using Game.Api.Services.Auth;
using Game.Api.Services.Player;
using Game.Api.Services.Config;
using Game.Api.Services.Room;
using Game.Api.Services.Match;
using Game.Api.Services.Session;
using Game.Api.Services.Runtime;
using Game.Api.Services.Settlement;
using Game.Api.Services.GameServer;
using Game.Api.Services.Inventory;
using Game.Api.Validators;
using FluentValidation;
using Game.Worker.ServerManager;
using Microsoft.Extensions.Diagnostics.HealthChecks;

namespace Game.Api.Extensions;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddGameInfrastructure(this IServiceCollection services, IConfiguration configuration)
    {
        var databaseOptions = configuration.GetSection(DatabaseOptions.Section).Get<DatabaseOptions>() ?? new();
        var redisOptions = configuration.GetSection(RedisOptions.Section).Get<RedisOptions>() ?? new();
        var jwtOptions = configuration.GetSection(JwtOptions.Section).Get<JwtOptions>() ?? new();

        ValidateRequiredInfrastructureOptions(databaseOptions, redisOptions, jwtOptions);

        services.AddDbContext<GameDbContext>(options =>
            options
                .UseNpgsql(databaseOptions.ConnectionString)
                .ConfigureWarnings(w => w.Ignore(Microsoft.EntityFrameworkCore.Diagnostics.CoreEventId.NavigationBaseIncludeIgnored)));

        services.AddSingleton<IRedisConnectionFactory>(_ =>
            new RedisConnectionFactory(redisOptions.ConnectionString));

        services.AddSingleton(jwtOptions);
        services.Configure<GameServerManagerOptions>(configuration.GetSection(GameServerManagerOptions.Section));
        services.AddScoped<IJwtTokenService, JwtTokenService>();

        services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
            .AddJwtBearer(options =>
            {
                options.TokenValidationParameters = new TokenValidationParameters
                {
                    ValidateIssuer = true,
                    ValidateAudience = true,
                    ValidateLifetime = true,
                    ValidateIssuerSigningKey = true,
                    ValidIssuer = jwtOptions.Issuer,
                    ValidAudience = jwtOptions.Audience,
                    IssuerSigningKey = new SymmetricSecurityKey(System.Text.Encoding.UTF8.GetBytes(jwtOptions.Secret))
                };
            });

        services.AddAuthorization();
        services.AddValidatorsFromAssemblyContaining<GuestLoginRequestValidator>();
        services.AddCors(options =>
        {
            options.AddDefaultPolicy(policy =>
            {
                policy
                    .WithOrigins(
                        configuration.GetSection("Cors:AllowedOrigins").Get<string[]>() ??
                        new[] { "http://localhost:3000", "http://localhost:8081", "http://tauri.localhost", "tauri://localhost" })
                    .AllowAnyHeader()
                    .AllowAnyMethod();
            });
        });

        // 注册应用服务 / Register application services
        services.AddScoped<Services.Auth.IAuthService, Services.Auth.AuthService>();
        services.AddScoped<IPlayerService, PlayerService>();
        services.AddScoped<Services.Config.IConfigService, ConfigService>();
        services.AddScoped<Services.Room.IRoomService, RoomService>();
        services.AddScoped<Services.Match.IMatchService, MatchService>();
        services.AddScoped<Services.Session.ISessionService, SessionService>();
        services.AddScoped<Services.Runtime.IGameServerService, GameServerService>();
        services.AddScoped<Services.Settlement.ISettlementService, SettlementService>();
        services.AddScoped<IGameServerManagerService, GameServerManagerService>();
        services.AddScoped<Services.Inventory.IInventoryService, InventoryService>();
        services.AddScoped<IServerManagerService, ServerManagerService>();

        return services;
    }

    public static IServiceCollection AddGameOpenTelemetry(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddOpenTelemetry()
            .ConfigureResource(r => r.AddService("GameApi"))
            .WithTracing(t => t
                .AddAspNetCoreInstrumentation()
                .AddHttpClientInstrumentation())
            .WithMetrics(m => m
                .AddAspNetCoreInstrumentation()
                .AddHttpClientInstrumentation()
                .AddPrometheusExporter());

        return services;
    }

    public static IServiceCollection AddGameSwagger(this IServiceCollection services, IConfiguration configuration)
    {
        var env = configuration["ASPNETCORE_ENVIRONMENT"];
        if (env != "Production")
        {
            services.AddEndpointsApiExplorer();
            services.AddSwaggerGen(c =>
            {
                // 配置XML文档以提取中文说明 / Configure XML docs for Chinese descriptions
                var xmlFile = Path.Combine(AppContext.BaseDirectory, "Game.Api.xml");
                if (File.Exists(xmlFile))
                {
                    c.IncludeXmlComments(xmlFile, true);
                }

                c.SwaggerDoc("v1", new OpenApiInfo
                {
                    Title = "游戏平台后端API",
                    Version = "v1",
                    Description = "MyGamePlatform 游戏后端接口文档 - 包含认证、玩家、房间、匹配、会话、结算等模块"
                });
                c.AddSecurityDefinition("Bearer", new OpenApiSecurityScheme
                {
                    Description = "JWT授权 header，使用Bearer方案。输入格式：Bearer {token}",
                    Name = "Authorization",
                    In = ParameterLocation.Header,
                    Type = SecuritySchemeType.ApiKey,
                    Scheme = "Bearer"
                });
                c.AddSecurityRequirement(new OpenApiSecurityRequirement
                {
                    {
                        new OpenApiSecurityScheme
                        {
                            Reference = new OpenApiReference
                            {
                                Type = ReferenceType.SecurityScheme,
                                Id = "Bearer"
                            }
                        },
                        Array.Empty<string>()
                    }
                });
            });
        }

        return services;
    }

    public static IServiceCollection AddGameHealthChecks(this IServiceCollection services, IConfiguration configuration)
    {
        var databaseOptions = configuration.GetSection(DatabaseOptions.Section).Get<DatabaseOptions>() ?? new();
        var redisOptions = configuration.GetSection(RedisOptions.Section).Get<RedisOptions>() ?? new();

        services.AddHealthChecks()
            .AddCheck("self", () => HealthCheckResult.Healthy(), tags: new[] { "live" })
            .AddNpgSql(databaseOptions.ConnectionString, name: "postgresql", tags: new[] { "ready" })
            .AddRedis(redisOptions.ConnectionString, name: "redis", tags: new[] { "ready" });

        return services;
    }

    private static void ValidateRequiredInfrastructureOptions(
        DatabaseOptions databaseOptions,
        RedisOptions redisOptions,
        JwtOptions jwtOptions)
    {
        if (string.IsNullOrWhiteSpace(databaseOptions.ConnectionString))
        {
            throw new InvalidOperationException("Database:ConnectionString must be configured.");
        }

        if (string.IsNullOrWhiteSpace(redisOptions.ConnectionString))
        {
            throw new InvalidOperationException("Redis:ConnectionString must be configured.");
        }

        if (string.IsNullOrWhiteSpace(jwtOptions.Secret) || jwtOptions.Secret.Length < 32)
        {
            throw new InvalidOperationException("Jwt:Secret must be configured with at least 32 characters.");
        }
    }
}
