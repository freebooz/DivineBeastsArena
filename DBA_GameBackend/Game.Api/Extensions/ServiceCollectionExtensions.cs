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
using Game.ServerManagement.DedicatedServers;
using Game.Application.Sessions;
using Game.Application.Characters;
using Game.Application.Auth;
using Game.Infrastructure.Database.Admissions;
using Game.Infrastructure.Database.Characters;
using Game.Infrastructure.Database.Sessions;
using Game.Infrastructure.Database.Auth;
using Game.Infrastructure.Security;
using Microsoft.Extensions.Diagnostics.HealthChecks;
using System.Threading.RateLimiting;

namespace Game.Api.Extensions;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddGameInfrastructure(
        this IServiceCollection services,
        IConfiguration configuration,
        IHostEnvironment? environment = null)
    {
        var databaseOptions = configuration.GetSection(DatabaseOptions.Section).Get<DatabaseOptions>() ?? new();
        var redisOptions = configuration.GetSection(RedisOptions.Section).Get<RedisOptions>() ?? new();
        var jwtOptions = configuration.GetSection(JwtOptions.Section).Get<JwtOptions>() ?? new();
        var dedicatedServerOptions = configuration.GetSection(DedicatedServerOrchestrationOptions.Section).Get<DedicatedServerOrchestrationOptions>() ?? new();
        var characterCreationOptions = configuration.GetSection(CharacterCreationOptions.Section).Get<CharacterCreationOptions>() ?? new();
        var sessionAdmissionOptions = configuration.GetSection(SessionAdmissionOptions.Section).Get<SessionAdmissionOptions>() ?? new();
        var villageSessionOptions = configuration.GetSection(VillageSessionOptions.Section).Get<VillageSessionOptions>() ?? new();
        var authenticationPolicyOptions = configuration.GetSection(AuthenticationPolicyOptions.Section).Get<AuthenticationPolicyOptions>() ?? new();
        if (string.IsNullOrWhiteSpace(authenticationPolicyOptions.PasswordResetBootstrapToken))
        {
            authenticationPolicyOptions.PasswordResetBootstrapToken =
                configuration["Auth:PasswordResetBootstrapToken"] ??
                configuration["PasswordReset:BootstrapToken"];
        }

        RequiredOptionsValidator.ValidateDatabase(databaseOptions);
        RequiredOptionsValidator.ValidateRedis(redisOptions);
        RequiredOptionsValidator.ValidateJwt(jwtOptions);
        RequiredOptionsValidator.ValidateInternalApiKey(configuration["InternalApi:Key"]);
        RequiredOptionsValidator.ValidateDedicatedServerOrchestration(
            dedicatedServerOptions,
            environment?.IsProduction() ?? false);
        RequiredOptionsValidator.ValidateCharacterCreation(characterCreationOptions);
        RequiredOptionsValidator.ValidateSessionAdmission(sessionAdmissionOptions);
        RequiredOptionsValidator.ValidateVillageSession(villageSessionOptions);
        RequiredOptionsValidator.ValidateAuthenticationPolicy(authenticationPolicyOptions);

        services.AddDbContext<GameDbContext>(options =>
            options
                .UseNpgsql(databaseOptions.ConnectionString)
                .ConfigureWarnings(w => w.Ignore(Microsoft.EntityFrameworkCore.Diagnostics.CoreEventId.NavigationBaseIncludeIgnored)));

        services.AddSingleton<IRedisConnectionFactory>(_ =>
            new RedisConnectionFactory(redisOptions.ConnectionString));

        services.AddSingleton(jwtOptions);
        services.AddSingleton(characterCreationOptions);
        services.AddSingleton(sessionAdmissionOptions);
        services.AddSingleton(authenticationPolicyOptions);
        services.AddSingleton<TimeProvider>(TimeProvider.System);
        services.Configure<DedicatedServerOrchestrationOptions>(configuration.GetSection(DedicatedServerOrchestrationOptions.Section));
        services.Configure<VillageSessionOptions>(configuration.GetSection(VillageSessionOptions.Section));
        services.AddSingleton<IJwtTokenService, JwtTokenService>();

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
        services.AddScoped<Services.Session.IVillageAllocationService, Services.Session.VillageAllocationService>();
        services.AddScoped<Services.Runtime.IGameServerService, GameServerService>();
        services.AddScoped<Services.Settlement.ISettlementService, SettlementService>();
        services.AddScoped<IGameServerRegistryService, GameServerRegistryService>();
        services.AddScoped<Services.Inventory.IInventoryService, InventoryService>();
        services.AddScoped<IDedicatedServerOrchestrator, DedicatedServerOrchestrator>();
        services.AddScoped<IJoinTicketStore, EfJoinTicketStore>();
        services.AddScoped<ISessionAdmissionStore, EfSessionAdmissionStore>();
        services.AddScoped<ISessionLifecycleStore, EfSessionLifecycleStore>();
        services.AddScoped<ISessionServerAllocationStore, EfSessionServerAllocationStore>();
        services.AddScoped<ISessionCreationStore, EfSessionCreationStore>();
        services.AddScoped<ISessionQueryStore, EfSessionQueryStore>();
        services.AddSingleton<ISessionCredentialIssuer, CryptographicSessionCredentialIssuer>();
        services.AddScoped<IIssueSessionConnectionUseCase, IssueSessionConnectionUseCase>();
        services.AddScoped<IChangeSessionLifecycleUseCase, ChangeSessionLifecycleUseCase>();
        services.AddScoped<IAllocateSessionServerUseCase, AllocateSessionServerUseCase>();
        services.AddScoped<ICreateSessionFromRoomUseCase, CreateSessionFromRoomUseCase>();
        services.AddScoped<ICreateSessionFromMatchUseCase, CreateSessionFromMatchUseCase>();
        services.AddScoped<IGetSessionUseCase, GetSessionUseCase>();
        services.AddScoped<IAuthenticatedAccountQueryStore, EfAuthenticatedAccountQueryStore>();
        services.AddScoped<IGetAuthenticatedAccountUseCase, GetAuthenticatedAccountUseCase>();
        services.AddScoped<IAuthenticationCredentialStore, EfAuthenticationCredentialStore>();
        services.AddSingleton<IPasswordCredentialVerifier, BcryptPasswordCredentialVerifier>();
        services.AddScoped<IAuthenticateCredentialsUseCase, AuthenticateCredentialsUseCase>();
        services.AddSingleton<ILoginCredentialIssuer, JwtLoginCredentialIssuer>();
        services.AddScoped<ILoginCredentialStore, EfLoginCredentialStore>();
        services.AddScoped<IIssueLoginCredentialsUseCase, IssueLoginCredentialsUseCase>();
        services.AddSingleton<IRefreshCredentialHasher, JwtRefreshCredentialHasher>();
        services.AddScoped<EfRefreshCredentialStore>();
        services.AddScoped<IRefreshCredentialRotationStore>(provider =>
            provider.GetRequiredService<EfRefreshCredentialStore>());
        services.AddScoped<ILogoutCredentialStore>(provider =>
            provider.GetRequiredService<EfRefreshCredentialStore>());
        services.AddScoped<IRotateRefreshCredentialUseCase, RotateRefreshCredentialUseCase>();
        services.AddScoped<ILogoutUseCase, LogoutUseCase>();
        services.AddScoped<IPasswordAccountStore, EfPasswordAccountStore>();
        services.AddSingleton<ISecureCredentialComparer, FixedTimeCredentialComparer>();
        services.AddScoped<IChangePasswordUseCase, ChangePasswordUseCase>();
        services.AddScoped<IResetPasswordUseCase, ResetPasswordUseCase>();
        services.AddSingleton<IDeviceIdentifierHasher, Sha256DeviceIdentifierHasher>();
        services.AddScoped<IAccountOnboardingStore, EfAccountOnboardingStore>();
        services.AddScoped<IGuestLoginUseCase, GuestLoginUseCase>();
        services.AddScoped<IRegisterAccountUseCase, RegisterAccountUseCase>();
        services.AddScoped<IConsumeJoinTicketUseCase, ConsumeJoinTicketUseCase>();
        services.AddScoped<IPlayerCharacterStore, EfPlayerCharacterStore>();
        services.AddSingleton<ICharacterBuildPolicy, CharacterBuildPolicy>();
        services.AddScoped<IGetPlayerCharactersUseCase, GetPlayerCharactersUseCase>();
        services.AddScoped<ICreatePlayerCharacterUseCase, CreatePlayerCharacterUseCase>();
        services.AddScoped<ISelectPlayerCharacterUseCase, SelectPlayerCharacterUseCase>();
        services.AddHostedService<Services.Match.MatchmakingBackgroundService>();

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

    public static IServiceCollection AddGameRateLimiting(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddRateLimiter(options =>
        {
            options.RejectionStatusCode = StatusCodes.Status429TooManyRequests;

            options.AddPolicy("auth", httpContext =>
                RateLimitPartition.GetFixedWindowLimiter(
                    partitionKey: BuildClientPartition(httpContext, "auth"),
                    factory: _ => new FixedWindowRateLimiterOptions
                    {
                        PermitLimit = configuration.GetValue("RateLimiting:Auth:PermitLimit", 20),
                        Window = TimeSpan.FromMinutes(configuration.GetValue("RateLimiting:Auth:WindowMinutes", 1)),
                        QueueLimit = 0,
                        AutoReplenishment = true
                    }));

            options.AddPolicy("admin-auth", httpContext =>
                RateLimitPartition.GetFixedWindowLimiter(
                    partitionKey: BuildClientPartition(httpContext, "admin-auth"),
                    factory: _ => new FixedWindowRateLimiterOptions
                    {
                        PermitLimit = configuration.GetValue("RateLimiting:AdminAuth:PermitLimit", 5),
                        Window = TimeSpan.FromMinutes(configuration.GetValue("RateLimiting:AdminAuth:WindowMinutes", 15)),
                        QueueLimit = 0,
                        AutoReplenishment = true
                    }));

            options.AddPolicy("admin", httpContext =>
                RateLimitPartition.GetFixedWindowLimiter(
                    partitionKey: BuildClientPartition(httpContext, "admin"),
                    factory: _ => new FixedWindowRateLimiterOptions
                    {
                        PermitLimit = configuration.GetValue("RateLimiting:Admin:PermitLimit", 120),
                        Window = TimeSpan.FromMinutes(configuration.GetValue("RateLimiting:Admin:WindowMinutes", 1)),
                        QueueLimit = 0,
                        AutoReplenishment = true
                    }));

            options.OnRejected = async (context, cancellationToken) =>
            {
                context.HttpContext.Response.ContentType = "application/json";
                await context.HttpContext.Response.WriteAsJsonAsync(
                    new { success = false, error = "RATE_LIMITED", message = "Too many requests. Please retry later." },
                    cancellationToken);
            };
        });

        return services;
    }

    public static IServiceCollection AddGameSwagger(this IServiceCollection services, IConfiguration configuration)
    {
        var env = configuration["ASPNETCORE_ENVIRONMENT"];
        var swaggerEnabled = configuration.GetValue<bool?>("Swagger:Enabled") ?? env != "Production";
        if (swaggerEnabled)
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
                    Description = "五灵争霸：神兽觉醒 游戏后端接口文档 - 包含认证、玩家、房间、匹配、会话、结算等模块"
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

    private static string BuildClientPartition(HttpContext httpContext, string policy)
    {
        var forwardedFor = httpContext.Request.Headers["X-Forwarded-For"].FirstOrDefault();
        var ip = string.IsNullOrWhiteSpace(forwardedFor)
            ? httpContext.Connection.RemoteIpAddress?.ToString()
            : forwardedFor.Split(',')[0].Trim();

        return $"{policy}:{ip ?? "unknown"}";
    }
}
