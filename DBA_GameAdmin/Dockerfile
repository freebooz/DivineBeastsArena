FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS base
WORKDIR /app
EXPOSE 8080

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src

# Copy DBA_GameAdmin project
COPY ["DBA_GameAdmin/GameAdmin.csproj", "DBA_GameAdmin/"]

# Copy Game.Shared from backend
COPY ["DBA_GameBackend/Game.Shared/Game.Shared.csproj", "Game.Shared/"]
COPY ["DBA_GameBackend/Game.Shared/", "Game.Shared/"]

# Copy DBA_GameAdmin source
COPY ["DBA_GameAdmin/", "DBA_GameAdmin/"]

RUN dotnet restore "DBA_GameAdmin/GameAdmin.csproj"
WORKDIR "/src/DBA_GameAdmin"
RUN dotnet build "GameAdmin.csproj" -c Release -o /app/build

FROM build AS publish
RUN dotnet publish "GameAdmin.csproj" -c Release -o /app/publish

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "GameAdmin.dll"]
