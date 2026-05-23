FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS base
WORKDIR /app

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src
COPY ["Game.Worker/Game.Worker.csproj", "Game.Worker/"]
COPY ["Game.ServerManagement/Game.ServerManagement.csproj", "Game.ServerManagement/"]
COPY ["Game.Shared/Game.Shared.csproj", "Game.Shared/"]
COPY ["Game.Infrastructure/Game.Infrastructure.csproj", "Game.Infrastructure/"]
COPY ["Game.Shared/", "Game.Shared/"]
COPY ["Game.Infrastructure/", "Game.Infrastructure/"]
COPY ["Game.ServerManagement/", "Game.ServerManagement/"]
COPY ["Game.Worker/", "Game.Worker/"]
RUN dotnet restore "Game.Worker/Game.Worker.csproj"
WORKDIR "/src/Game.Worker"
RUN dotnet build "Game.Worker.csproj" -c Release -o /app/build

FROM build AS publish
RUN dotnet publish "Game.Worker.csproj" -c Release -o /app/publish

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "Game.Worker.dll"]
