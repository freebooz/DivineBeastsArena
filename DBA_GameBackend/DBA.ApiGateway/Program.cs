using System.Diagnostics;
using System.Threading.RateLimiting;
using OpenTelemetry.Metrics;
using OpenTelemetry.Resources;
using OpenTelemetry.Trace;
using Microsoft.AspNetCore.RateLimiting;
using Serilog;
using Serilog.Context;
using Serilog.Formatting.Compact;

Log.Logger = new LoggerConfiguration()
    .MinimumLevel.Information()
    .WriteTo.Console(new CompactJsonFormatter())
    .CreateLogger();

try
{
    var builder = WebApplication.CreateBuilder(args);
    builder.Host.UseSerilog();

    builder.Services.AddReverseProxy()
        .LoadFromConfig(builder.Configuration.GetSection("ReverseProxy"));
    builder.Services.AddHealthChecks()
        .AddCheck("self", () => Microsoft.Extensions.Diagnostics.HealthChecks.HealthCheckResult.Healthy(), tags: new[] { "live", "ready" });
    builder.Services.AddOpenTelemetry()
        .ConfigureResource(resource => resource.AddService("DBA.ApiGateway"))
        .WithTracing(tracing => tracing
            .AddAspNetCoreInstrumentation()
            .AddHttpClientInstrumentation())
        .WithMetrics(metrics => metrics
            .AddAspNetCoreInstrumentation()
            .AddHttpClientInstrumentation()
            .AddPrometheusExporter());
    builder.Services.AddRateLimiter(options =>
    {
        options.RejectionStatusCode = StatusCodes.Status429TooManyRequests;
        options.AddFixedWindowLimiter("gateway", limiterOptions =>
        {
            limiterOptions.PermitLimit = builder.Configuration.GetValue("RateLimiting:PermitLimit", 120);
            limiterOptions.Window = TimeSpan.FromMinutes(builder.Configuration.GetValue("RateLimiting:WindowMinutes", 1));
            limiterOptions.QueueLimit = 0;
            limiterOptions.AutoReplenishment = true;
        });
    });

    var app = builder.Build();

    app.Use(async (context, next) =>
    {
        var correlationId = context.Request.Headers["X-Trace-Id"].FirstOrDefault();
        if (string.IsNullOrWhiteSpace(correlationId))
        {
            correlationId = Activity.Current?.TraceId.ToString() ?? Guid.NewGuid().ToString("N");
        }

        context.TraceIdentifier = correlationId;
        context.Response.Headers["X-Trace-Id"] = correlationId;
        using (LogContext.PushProperty("CorrelationId", correlationId))
        {
            await next();
        }
    });

    app.UseRateLimiter();
    app.MapHealthChecks("/health/live", new Microsoft.AspNetCore.Diagnostics.HealthChecks.HealthCheckOptions
    {
        Predicate = check => check.Tags.Contains("live")
    });
    app.MapHealthChecks("/health/ready", new Microsoft.AspNetCore.Diagnostics.HealthChecks.HealthCheckOptions
    {
        Predicate = check => check.Tags.Contains("ready")
    });
    app.MapPrometheusScrapingEndpoint("/metrics");
    app.MapReverseProxy().RequireRateLimiting("gateway");

    Log.Information("DBA ApiGateway 正在启动，业务请求将由 YARP 转发。");
    await app.RunAsync();
}
catch (Exception exception)
{
    Log.Fatal(exception, "DBA ApiGateway 启动失败。");
}
finally
{
    Log.CloseAndFlush();
}

public partial class Program;
