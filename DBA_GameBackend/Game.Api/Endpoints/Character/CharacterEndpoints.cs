using Game.Application.Characters;
using Game.Api.Extensions;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Character;

public static class CharacterEndpoints
{
    public static void MapCharacterEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/v1/characters").WithTags("CharacterService").RequireAuthorization();
        group.MapGet("", GetRoster);
        group.MapGet("/{characterId:guid}", GetCharacter);
        group.MapPost("", CreateCharacter);
        group.MapDelete("/{characterId:guid}", DeleteCharacter);
        group.MapPost("/{characterId:guid}/select", SelectCharacter);
    }

    private static async Task<IResult> GetRoster(HttpContext context, Guid serverId, ICharacterRosterService service, CancellationToken ct) =>
        ToResult(await service.GetRosterAsync(GetAccountId(context), serverId, ct));
    private static async Task<IResult> GetCharacter(HttpContext context, Guid characterId, ICharacterRosterService service, CancellationToken ct) =>
        ToResult(await service.GetAsync(GetAccountId(context), characterId, ct));
    private static async Task<IResult> SelectCharacter(HttpContext context, Guid characterId, ICharacterRosterService service, CancellationToken ct) =>
        ToResult(await service.SelectAsync(GetAccountId(context), characterId, ct));
    private static async Task<IResult> CreateCharacter(HttpContext context, CreateCharacterRequest request, ICharacterRosterService service, CancellationToken ct)
    {
        context.Request.Headers.TryGetValue("Idempotency-Key", out var key);
        return ToResult(await service.CreateAsync(new CreateCharacterCommand(GetAccountId(context), request.ServerId, request.Name, request.ZodiacType, request.ElementType, request.FiveCampType, request.Appearance?.ToPayload(), key.FirstOrDefault()), ct));
    }
    private static async Task<IResult> DeleteCharacter(HttpContext context, Guid characterId, ICharacterRosterService service, CancellationToken ct)
    {
        if (!context.Request.Headers.TryGetValue("X-Character-Delete-Confirm", out var confirmed) || !string.Equals(confirmed, "true", StringComparison.OrdinalIgnoreCase))
            return ErrorResponse.Create(422, "角色请求失败", "删除角色需要二次确认。", code: "CHARACTER_DELETE_CONFIRM_REQUIRED").ToProblem();
        return ToResult(await service.DeleteAsync(GetAccountId(context), characterId, ct));
    }

    private static Guid GetAccountId(HttpContext context) => Guid.TryParse(context.User.FindFirst("player_id")?.Value, out var id) ? id : Guid.Empty;
    private static IResult ToResult<T>(CharacterServiceResult<T> result) where T : class => result.Success
        ? Results.Ok(ApiResponse<T>.Ok(result.Value!))
        : ErrorResponse.Create(StatusFor(result.ErrorCode), "角色请求失败", result.ErrorMessage, code: result.ErrorCode).ToProblem();
    private static int StatusFor(string code) => code is "CHARACTER_NOT_FOUND" or "SERVER_NOT_FOUND" ? 404 : code is "CHARACTER_NAME_DUPLICATE" ? 409 : 422;

    public sealed record CreateCharacterRequest(Guid ServerId, string? Name, string? ZodiacType, string? ElementType, string? FiveCampType, AppearanceRequest? Appearance);
    public sealed record AppearanceRequest(Dictionary<string, string>? OptionIds, List<string>? EquipmentVisualIds, string? WeaponVisualId, string? SkinId)
    { public CharacterAppearancePayload ToPayload() => new(OptionIds, EquipmentVisualIds, WeaponVisualId, SkinId); }
}
