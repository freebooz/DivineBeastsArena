FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS base
WORKDIR /app
EXPOSE 8080
RUN apt-get update \
    && apt-get install -y --no-install-recommends curl \
    && rm -rf /var/lib/apt/lists/*

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src
COPY ["Game.Api/Game.Api.csproj", "Game.Api/"]
COPY ["Game.Worker/Game.Worker.csproj", "Game.Worker/"]
COPY ["Game.Shared/Game.Shared.csproj", "Game.Shared/"]
COPY ["Game.Infrastructure/Game.Infrastructure.csproj", "Game.Infrastructure/"]
COPY ["Game.Shared/", "Game.Shared/"]
COPY ["Game.Infrastructure/", "Game.Infrastructure/"]
COPY ["Game.Worker/", "Game.Worker/"]
COPY ["Game.Api/", "Game.Api/"]
RUN dotnet restore "Game.Api/Game.Api.csproj"
WORKDIR "/src/Game.Api"
RUN dotnet build "Game.Api.csproj" -c Release -o /app/build

FROM build AS publish
RUN dotnet publish "Game.Api.csproj" -c Release -o /app/publish

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "Game.Api.dll"]
