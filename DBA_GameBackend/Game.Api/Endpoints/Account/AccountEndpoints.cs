/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Security.Claims;
using System.Text.Json;
using System.Text.RegularExpressions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Account;

public static class AccountEndpoints
{
    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web);
    private static readonly Regex CharacterNameRegex = new(@"^[\u4e00-\u9fa5A-Za-z0-9_]{2,16}$", RegexOptions.Compiled);

    public static void MapAccountEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/account").WithTags("客户端账号兼容").RequireAuthorization();

        var playerGroup = app.MapGroup("/api/players/me/characters").WithTags("Player Characters").RequireAuthorization();

        group.MapGet("/characters", GetCharacters)
            .WithSummary("获取当前账号角色列表")
            .WithDescription("兼容旧版 Unreal 客户端的角色列表接口。");

        group.MapPost("/characters", CreateCharacter)
            .WithSummary("创建当前账号角色")
            .WithDescription("兼容旧版 Unreal 客户端的创建角色接口。");

        group.MapPost("/characters/{characterId}/select", SelectCharacter)
            .WithSummary("选择当前账号角色")
            .WithDescription("将指定角色标记为当前选中角色并写入数据库。");

        group.MapPost("/characters/select", SelectCharacterByBody)
            .WithSummary("选择当前账号角色")
            .WithDescription("兼容只支持 JSON Body 的客户端，写入当前选中角色。");
        playerGroup.MapGet("", GetCharacters)
            .WithSummary("Get current player's characters");
        playerGroup.MapPost("", CreateCharacter)
            .WithSummary("Create current player's character");
        playerGroup.MapPost("/{characterId}/select", SelectCharacter)
            .WithSummary("Select current player's character");
        playerGroup.MapPost("/select", SelectCharacterByBody)
            .WithSummary("Select current player's character by JSON body");
    }

    private static async Task<IResult> GetCharacters(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null)
            return Results.Ok(new { success = false, error = "Unauthorized" });

        var rows = await db.PlayerCharacters
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.IsSelected)
            .ThenByDescending(x => x.LastUsedAt)
            .ThenBy(x => x.CreatedAt)
            .ToListAsync();

        var characters = rows
            .Select(ToCharacterSummary)
            .ToArray();
        var selectedCharacterId = rows.FirstOrDefault(x => x.IsSelected)?.Id.ToString("N") ?? string.Empty;

        return Results.Ok(new { success = true, selectedCharacterId, characters });
    }

    private static async Task<IResult> CreateCharacter(HttpContext ctx, GameDbContext db, CreateCharacterRequest request)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null)
            return Results.Ok(new { success = false, error = "Unauthorized" });

        var characterName = string.IsNullOrWhiteSpace(request.CharacterName)
            ? $"Hero_{Guid.NewGuid():N}"[..13]
            : request.CharacterName.Trim();

        if (!CharacterNameRegex.IsMatch(characterName))
            return Results.Ok(new { success = false, error = "角色名需为 2-16 位，仅允许中文、英文、数字和下划线。" });

        var duplicate = await db.PlayerCharacters.AnyAsync(x => x.PlayerId == playerId.Value && x.CharacterName == characterName);
        if (duplicate)
            return Results.Ok(new { success = false, error = "Character name already exists." });

        var now = DateTimeOffset.UtcNow;
        var row = new PlayerCharacter
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId.Value,
            CharacterName = characterName,
            Zodiac = NormalizeChoice(request.Zodiac, "Rat"),
            PrimaryElement = NormalizeChoice(request.PrimaryElement, "Water"),
            FiveCamp = NormalizeChoice(request.FiveCamp, "East"),
            FixedSkillGroupId = BuildSkillGroupId(request.Zodiac, request.PrimaryElement),
            CoreAttributesJson = JsonSerializer.Serialize(new CharacterCoreAttributes(1800, 100, 40, 380, 100, 10, 0.05f, 2.0f), JsonOptions),
            Level = 1,
            IsSelected = false,
            CreatedAt = now,
            LastUsedAt = now
        };

        db.PlayerCharacters.Add(row);
        await db.SaveChangesAsync();

        return Results.Ok(new { success = true, character = ToCharacterSummary(row) });
    }

    private static async Task<IResult> SelectCharacter(HttpContext ctx, GameDbContext db, string characterId)
    {
        return await SelectCharacterCore(ctx, db, characterId);
    }

    private static async Task<IResult> SelectCharacterByBody(HttpContext ctx, GameDbContext db, SelectCharacterRequest request)
    {
        return await SelectCharacterCore(ctx, db, request.CharacterId);
    }

    private static async Task<IResult> SelectCharacterCore(HttpContext ctx, GameDbContext db, string? characterId)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null)
            return Results.Ok(new { success = false, error = "Unauthorized" });

        if (!Guid.TryParse(characterId, out var parsedId))
            return Results.Ok(new { success = false, error = "Invalid character id." });

        await using var tx = await db.Database.BeginTransactionAsync();

        var characters = await db.PlayerCharacters
            .Where(x => x.PlayerId == playerId.Value)
            .ToListAsync();
        var selected = characters.FirstOrDefault(x => x.Id == parsedId);
        if (selected == null)
            return Results.Ok(new { success = false, error = "Character not found." });

        var now = DateTimeOffset.UtcNow;
        foreach (var character in characters)
        {
            character.IsSelected = character.Id == parsedId;
            if (character.IsSelected)
            {
                character.LastUsedAt = now;
                character.UpdatedAt = now;
            }
        }

        await db.SaveChangesAsync();
        await tx.CommitAsync();

        return Results.Ok(new { success = true, selectedCharacterId = selected.Id.ToString("N"), character = ToCharacterSummary(selected) });
    }

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id") ?? ctx.User.FindFirst(ClaimTypes.NameIdentifier);
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static CharacterSummary ToCharacterSummary(PlayerCharacter character)
    {
        var attributes = TryReadCoreAttributes(character.CoreAttributesJson);
        return new CharacterSummary(
            character.Id.ToString("N"),
            character.CharacterName,
            character.Zodiac,
            character.PrimaryElement,
            character.FiveCamp,
            character.FixedSkillGroupId,
            attributes,
            character.Level,
            character.CreatedAt.ToUnixTimeSeconds(),
            character.LastUsedAt.ToUnixTimeSeconds());
    }

    private static CharacterCoreAttributes TryReadCoreAttributes(string json)
    {
        try
        {
            return JsonSerializer.Deserialize<CharacterCoreAttributes>(json, JsonOptions)
                ?? new CharacterCoreAttributes(1800, 100, 40, 380, 100, 10, 0.05f, 2.0f);
        }
        catch
        {
            return new CharacterCoreAttributes(1800, 100, 40, 380, 100, 10, 0.05f, 2.0f);
        }
    }

    private static string NormalizeChoice(string? value, string fallback)
    {
        return string.IsNullOrWhiteSpace(value) || value.Equals("None", StringComparison.OrdinalIgnoreCase)
            ? fallback
            : value.Trim();
    }

    private static string BuildSkillGroupId(string? zodiac, string? element)
    {
        var normalizedZodiac = NormalizeChoice(zodiac, "Rat");
        var normalizedElement = NormalizeChoice(element, "Water");
        return $"{normalizedZodiac}_{normalizedElement}_Default";
    }

    public sealed record CreateCharacterRequest(
        string? CharacterName,
        string? Zodiac,
        string? PrimaryElement,
        string? FiveCamp);

    public sealed record SelectCharacterRequest(string? CharacterId);

    public sealed record CharacterSummary(
        string CharacterId,
        string CharacterName,
        string Zodiac,
        string PrimaryElement,
        string FiveCamp,
        string FixedSkillGroupId,
        CharacterCoreAttributes CoreAttributes,
        int Level,
        long CreateTime,
        long LastUsedTime);

    public sealed record CharacterCoreAttributes(
        float MaxHealth,
        float AttackPower,
        float Defense,
        float MoveSpeed,
        float MaxEnergy,
        float EnergyRegen,
        float CriticalRate,
        float CriticalMultiplier);
}
