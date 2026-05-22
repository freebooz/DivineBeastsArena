/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：集中定义请求参数校验规则，阻止非法输入进入业务服务层。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using FluentValidation;
using Game.Shared.Contracts.Auth;
using Game.Shared.Contracts.Config;
using Game.Shared.Contracts.Inventory;
using Game.Shared.Contracts.Match;
using Game.Shared.Contracts.Player;
using Game.Shared.Contracts.Room;
using SettlementSubmitMatchResultRequest = Game.Shared.Contracts.Settlement.SubmitMatchResultRequest;

namespace Game.Api.Validators;

public sealed class GuestLoginRequestValidator : AbstractValidator<GuestLoginRequest>
{
    public GuestLoginRequestValidator()
    {
        RuleFor(x => x.DeviceId).MaximumLength(128);
        RuleFor(x => x.DeviceName).MaximumLength(64);
        RuleFor(x => x.Platform).MaximumLength(32);
    }
}

public sealed class DevLoginRequestValidator : AbstractValidator<DevLoginRequest>
{
    public DevLoginRequestValidator()
    {
        RuleFor(x => x.Username ?? x.DisplayName)
            .NotEmpty()
            .MaximumLength(32);
        RuleFor(x => x.Password).MaximumLength(128);
    }
}

public sealed class RefreshTokenRequestValidator : AbstractValidator<RefreshTokenRequest>
{
    public RefreshTokenRequestValidator()
    {
        RuleFor(x => x.RefreshToken).NotEmpty().MaximumLength(512);
    }
}

public sealed class UpdateProfileRequestValidator : AbstractValidator<UpdateProfileRequest>
{
    public UpdateProfileRequestValidator()
    {
        RuleFor(x => x.Nickname)
            .Length(2, 16)
            .Matches(@"^[\u4e00-\u9fa5A-Za-z0-9_]+$")
            .When(x => !string.IsNullOrWhiteSpace(x.Nickname));
        RuleFor(x => x.Avatar).MaximumLength(512);
    }
}

public sealed class UpdateSettingsRequestValidator : AbstractValidator<UpdateSettingsRequest>
{
    public UpdateSettingsRequestValidator()
    {
        RuleFor(x => x.Settings).NotNull();
        RuleFor(x => x.Settings.Count).LessThanOrEqualTo(128);
    }
}

public sealed class CreateConfigRequestValidator : AbstractValidator<CreateConfigRequest>
{
    private static readonly HashSet<string> AllowedKeys = new(StringComparer.OrdinalIgnoreCase)
    {
        "zodiac_character", "element_skill", "god_skill", "map_config",
        "match_mode", "reward_table", "item_table", "bot_config"
    };

    public CreateConfigRequestValidator()
    {
        RuleFor(x => x.ConfigKey).Must(x => AllowedKeys.Contains(x)).WithMessage("config_key is not allowed");
        RuleFor(x => x.Version).NotEmpty().MaximumLength(32);
        RuleFor(x => x.ContentJson).NotEmpty().Must(IsJson).WithMessage("content_json must be valid JSON");
        RuleFor(x => x.Channel).NotEmpty().MaximumLength(32);
        RuleFor(x => x.Region).NotEmpty().MaximumLength(32);
    }

    private static bool IsJson(string value)
    {
        try
        {
            using var _ = System.Text.Json.JsonDocument.Parse(value);
            return true;
        }
        catch
        {
            return false;
        }
    }
}

public sealed class UpdateConfigRequestValidator : AbstractValidator<UpdateConfigRequest>
{
    public UpdateConfigRequestValidator()
    {
        RuleFor(x => x.ContentJson).NotEmpty().Must(value =>
        {
            try
            {
                using var _ = System.Text.Json.JsonDocument.Parse(value);
                return true;
            }
            catch
            {
                return false;
            }
        });
    }
}

public sealed class CreateRoomRequestValidator : AbstractValidator<CreateRoomRequest>
{
    public CreateRoomRequestValidator()
    {
        RuleFor(x => x.Mode).NotEmpty().MaximumLength(32);
        RuleFor(x => x.MapId).NotEmpty().MaximumLength(64);
        RuleFor(x => x.Region).NotEmpty().MaximumLength(32);
        RuleFor(x => x.MaxPlayers).InclusiveBetween(1, 64);
        RuleFor(x => x.Visibility).Must(x => x is "PUBLIC" or "PRIVATE").WithMessage("visibility must be PUBLIC or PRIVATE");
    }
}

public sealed class CreateMatchmakingTicketRequestValidator : AbstractValidator<CreateMatchmakingTicketRequest>
{
    public CreateMatchmakingTicketRequestValidator()
    {
        RuleFor(x => x.Mode).NotEmpty().MaximumLength(32);
        RuleFor(x => x.Region).NotEmpty().MaximumLength(32);
        RuleFor(x => x.Mmr).InclusiveBetween(0, 10000);
    }
}

public sealed class GrantItemRequestValidator : AbstractValidator<GrantItemRequest>
{
    public GrantItemRequestValidator()
    {
        RuleFor(x => x.PlayerId).NotEmpty();
        RuleFor(x => x.ItemId).NotEmpty().MaximumLength(64);
        RuleFor(x => x.Quantity).GreaterThan(0);
        RuleFor(x => x.Reason).NotEmpty().MaximumLength(256);
    }
}

public sealed class DeductItemRequestValidator : AbstractValidator<DeductItemRequest>
{
    public DeductItemRequestValidator()
    {
        RuleFor(x => x.PlayerId).NotEmpty();
        RuleFor(x => x.ItemId).NotEmpty().MaximumLength(64);
        RuleFor(x => x.Quantity).GreaterThan(0);
        RuleFor(x => x.Reason).NotEmpty().MaximumLength(256);
    }
}

public sealed class SubmitMatchResultRequestValidator : AbstractValidator<SettlementSubmitMatchResultRequest>
{
    public SubmitMatchResultRequestValidator()
    {
        RuleFor(x => x.SessionId).NotEmpty();
        RuleFor(x => x.IdempotencyKey).NotEmpty().MaximumLength(128);
        RuleFor(x => x.ResultJson).NotEmpty();
        RuleFor(x => x.Players).NotEmpty();
    }
}
