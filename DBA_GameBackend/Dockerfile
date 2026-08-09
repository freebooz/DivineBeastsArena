# DBA_GameBackend - 统一Docker镜像
# 包含 Game.Api 和 Game.Worker

FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS base
WORKDIR /app
EXPOSE 8080

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src

# 复制解决方案和项目文件
COPY GameBackend.slnx ./
COPY Game.Api/Game.Api.csproj Game.Api/
COPY Game.Worker/Game.Worker.csproj Game.Worker/
COPY DBA.ApiGateway/DBA.ApiGateway.csproj DBA.ApiGateway/
COPY Game.Application/Game.Application.csproj Game.Application/
COPY Game.Shared/Game.Shared.csproj Game.Shared/
COPY Game.Infrastructure/Game.Infrastructure.csproj Game.Infrastructure/
COPY Game.Api.Tests/Game.Api.Tests.csproj Game.Api.Tests/
COPY Game.IntegrationTests/Game.IntegrationTests.csproj Game.IntegrationTests/
COPY Game.ServerManagement/Game.ServerManagement.csproj Game.ServerManagement/
COPY Game.ServerManagement.Tests/Game.ServerManagement.Tests.csproj Game.ServerManagement.Tests/

# 还原依赖
RUN dotnet restore

# 复制所有源代码
COPY . .

# 构建所有项目
RUN dotnet build GameBackend.slnx -c Release --no-restore

# ========== Game.Api 发布 ==========
FROM build AS publish-api
RUN dotnet publish Game.Api/Game.Api.csproj -c Release -o /app/publish/api --no-build

# ========== Game.Worker 发布 ==========
FROM build AS publish-worker
RUN dotnet publish Game.Worker/Game.Worker.csproj -c Release -o /app/publish/worker --no-build

# ========== 最终镜像 ==========
FROM base AS final

WORKDIR /app

# 复制API和Worker
COPY --from=publish-api /app/publish/api/ ./api/
COPY --from=publish-worker /app/publish/worker/ ./worker/

# 创建启动脚本
RUN echo '#!/bin/bash' > /app/start.sh && \
    echo 'echo "Starting DBA_GameBackend services..."' >> /app/start.sh && \
    echo '# 启动 Worker 服务（后台运行）' >> /app/start.sh && \
    echo 'dotnet /app/worker/Game.Worker.dll &' >> /app/start.sh && \
    echo 'WORKER_PID=$!' >> /app/start.sh && \
    echo '' >> /app/start.sh && \
    echo '# 等待Worker启动' >> /app/start.sh && \
    echo 'sleep 3' >> /app/start.sh && \
    echo '' >> /app/start.sh && \
    echo '# 启动 API 服务' >> /app/start.sh && \
    echo 'echo "Starting Game.Api on port 8080..."' >> /app/start.sh && \
    echo 'dotnet /app/api/Game.Api.dll' >> /app/start.sh && \
    echo '' >> /app/start.sh && \
    echo '# 当API停止时，停止Worker' >> /app/start.sh && \
    echo 'kill $WORKER_PID 2>/dev/null' >> /app/start.sh && \
    chmod +x /app/start.sh

# 健康检查
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health/live || exit 1

# 默认启动脚本
ENTRYPOINT ["/app/start.sh"]
