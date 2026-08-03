/*
中文阅读说明：
- 所属应用：DBA_GameBackend API 表现层。
- 文件职责：声明玩家角色 HTTP 路由，读取鉴权身份并将应用层结果映射为兼容响应。
- 架构约束：本文件不得直接访问 GameDbContext、EF 实体或承载角色创建与选择规则。
*/

using Game.Api.Extensions;
using Game.Application.Characters;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Account;

public static class AccountEndpoints
{
    public static void MapAccountEndpoints(this IEndpointRouteBuilder app)
    {
        var legacyGroup = app.MapGroup("/api/account")
            .WithTags("客户端账号兼容")
            .RequireAuthorization();
        var playerGroup = app.MapGroup("/api/players/me/characters")
            .WithTags("玩家角色")
            .RequireAuthorization();

        legacyGroup.MapGet("/characters", GetCharacters)
            .WithSummary("获取当前账号角色列表")
            .WithDescription("兼容旧版 Unreal 客户端的角色列表接口。");
        legacyGroup.MapPost("/characters", CreateCharacter)
            .WithSummary("创建当前账号角色")
            .WithDescription("兼容旧版 Unreal 客户端的创建角色接口。");
        legacyGroup.MapPost("/characters/{characterId}/select", SelectCharacter)
            .WithSummary("选择当前账号角色")
            .WithDescription("将指定角色标记为当前选中角色。");
        legacyGroup.MapPost("/characters/select", SelectCharacterByBody)
            .WithSummary("选择当前账号角色")
            .WithDescription("兼容只支持 JSON Body 的客户端。");

        playerGroup.MapGet("", GetCharacters)
            .WithSummary("获取当前玩家角色列表");
        playerGroup.MapPost("", CreateCharacter)
            .WithSummary("创建当前玩家角色");
        playerGroup.MapPost("/{characterId}/select", SelectCharacter)
            .WithSummary("选择当前玩家角色");
        playerGroup.MapPost("/select", SelectCharacterByBody)
            .WithSummary("通过请求体选择当前玩家角色");
    }

    private static async Task<IResult> GetCharacters(
        HttpContext context,
        IGetPlayerCharactersUseCase useCase,
        CancellationToken cancellationToken)
    {
        var playerId = GetPlayerId(context);
        if (playerId is null)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var result = await useCase.ExecuteAsync(playerId.Value, cancellationToken);
        if (!result.Success || result.Value is null)
        {
            return Results.Ok(new CharacterFailureResponse(false, result.ErrorCode, result.ErrorMessage));
        }

        var roster = result.Value;
        return Results.Ok(new CharacterRosterResponse(
            true,
            roster.SelectedCharacterId?.ToString("N") ?? string.Empty,
            roster.Characters.Select(ToResponse).ToArray()));
    }

    private static async Task<IResult> CreateCharacter(
        HttpContext context,
        CreateCharacterRequest request,
        ICreatePlayerCharacterUseCase useCase,
        CancellationToken cancellationToken)
    {
        var playerId = GetPlayerId(context);
        if (playerId is null)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var result = await useCase.ExecuteAsync(
            new CreatePlayerCharacterCommand(
                playerId.Value,
                request.CharacterName,
                request.Zodiac,
                request.PrimaryElement,
                request.FiveCamp),
            cancellationToken);
        return !result.Success || result.Value is null
            ? Results.Ok(new CharacterFailureResponse(false, result.ErrorCode, result.ErrorMessage))
            : Results.Ok(new CharacterMutationResponse(true, string.Empty, ToResponse(result.Value)));
    }

    private static Task<IResult> SelectCharacter(
        HttpContext context,
        string characterId,
        ISelectPlayerCharacterUseCase useCase,
        CancellationToken cancellationToken)
    {
        return SelectCharacterCore(context, characterId, useCase, cancellationToken);
    }

    private static Task<IResult> SelectCharacterByBody(
        HttpContext context,
        SelectCharacterRequest request,
        ISelectPlayerCharacterUseCase useCase,
        CancellationToken cancellationToken)
    {
        return SelectCharacterCore(context, request.CharacterId, useCase, cancellationToken);
    }

    private static async Task<IResult> SelectCharacterCore(
        HttpContext context,
        string? characterId,
        ISelectPlayerCharacterUseCase useCase,
        CancellationToken cancellationToken)
    {
        var playerId = GetPlayerId(context);
        if (playerId is null)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var result = await useCase.ExecuteAsync(playerId.Value, characterId, cancellationToken);
        return !result.Success || result.Value is null
            ? Results.Ok(new CharacterFailureResponse(false, result.ErrorCode, result.ErrorMessage))
            : Results.Ok(new CharacterMutationResponse(
                true,
                result.Value.CharacterId.ToString("N"),
                ToResponse(result.Value)));
    }

    private static Guid? GetPlayerId(HttpContext context)
    {
        var claim = context.User.FindFirst("player_id");
        return claim is not null && Guid.TryParse(claim.Value, out var playerId)
            ? playerId
            : null;
    }

    private static CharacterSummaryResponse ToResponse(PlayerCharacterSummary character)
    {
        var attributes = character.CoreAttributes;
        return new CharacterSummaryResponse(
            character.CharacterId.ToString("N"),
            character.CharacterName,
            character.Zodiac,
            character.PrimaryElement,
            character.FiveCamp,
            character.FixedSkillGroupId,
            new CharacterCoreAttributesResponse(
                attributes.MaxHealth,
                attributes.AttackPower,
                attributes.Defense,
                attributes.MoveSpeed,
                attributes.MaxEnergy,
                attributes.EnergyRegen,
                attributes.CriticalRate,
                attributes.CriticalMultiplier),
            character.Level,
            character.CreatedAt.ToUnixTimeSeconds(),
            character.LastUsedAt.ToUnixTimeSeconds());
    }

    public sealed record CreateCharacterRequest(
        string? CharacterName,
        string? Zodiac,
        string? PrimaryElement,
        string? FiveCamp);

    public sealed record SelectCharacterRequest(string? CharacterId);

    public sealed record CharacterFailureResponse(bool Success, string Code, string Error);

    public sealed record CharacterRosterResponse(
        bool Success,
        string SelectedCharacterId,
        IReadOnlyList<CharacterSummaryResponse> Characters);

    public sealed record CharacterMutationResponse(
        bool Success,
        string SelectedCharacterId,
        CharacterSummaryResponse Character);

    public sealed record CharacterSummaryResponse(
        string CharacterId,
        string CharacterName,
        string Zodiac,
        string PrimaryElement,
        string FiveCamp,
        string FixedSkillGroupId,
        CharacterCoreAttributesResponse CoreAttributes,
        int Level,
        long CreateTime,
        long LastUsedTime);

    public sealed record CharacterCoreAttributesResponse(
        float MaxHealth,
        float AttackPower,
        float Defense,
        float MoveSpeed,
        float MaxEnergy,
        float EnergyRegen,
        float CriticalRate,
        float CriticalMultiplier);
}
