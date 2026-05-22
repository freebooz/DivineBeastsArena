/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：测试代码，用于锁定关键业务契约，防止后续重构破坏已有行为。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Api.Validators;
using Game.Shared.Contracts.Config;
using Game.Shared.Contracts.Player;
using Game.Shared.Contracts.Room;

namespace Game.Api.Tests;

public class ModuleValidatorTests
{
    [Fact]
    public void PlayerNickname_AllowsChineseEnglishDigitsAndUnderscore()
    {
        var validator = new UpdateProfileRequestValidator();
        var result = validator.Validate(new UpdateProfileRequest("神兽_Player01", null));

        Assert.True(result.IsValid);
    }

    [Fact]
    public void PlayerNickname_RejectsSymbols()
    {
        var validator = new UpdateProfileRequestValidator();
        var result = validator.Validate(new UpdateProfileRequest("bad-name!", null));

        Assert.False(result.IsValid);
    }

    [Fact]
    public void ConfigValidator_RejectsUnknownConfigKey()
    {
        var validator = new CreateConfigRequestValidator();
        var result = validator.Validate(new CreateConfigRequest(
            "unknown_table",
            "1",
            "{}",
            "default",
            "global",
            null,
            null));

        Assert.False(result.IsValid);
    }

    [Fact]
    public void RoomValidator_AllowsPublicRoom()
    {
        var validator = new CreateRoomRequestValidator();
        var result = validator.Validate(new CreateRoomRequest(
            "classic",
            "arena_01",
            "cn",
            8,
            "PUBLIC",
            null));

        Assert.True(result.IsValid);
    }
}
