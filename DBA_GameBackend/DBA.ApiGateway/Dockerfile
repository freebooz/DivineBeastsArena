FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS base
WORKDIR /app
EXPOSE 8080
RUN apt-get update \
    && apt-get install -y --no-install-recommends curl \
    && rm -rf /var/lib/apt/lists/*

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src
COPY ["DBA.ApiGateway/DBA.ApiGateway.csproj", "DBA.ApiGateway/"]
RUN dotnet restore "DBA.ApiGateway/DBA.ApiGateway.csproj"
COPY ["DBA.ApiGateway/", "DBA.ApiGateway/"]
WORKDIR "/src/DBA.ApiGateway"
RUN dotnet publish "DBA.ApiGateway.csproj" -c Release -o /app/publish --no-restore

FROM base AS final
WORKDIR /app
COPY --from=build /app/publish .
ENTRYPOINT ["dotnet", "DBA.ApiGateway.dll"]
