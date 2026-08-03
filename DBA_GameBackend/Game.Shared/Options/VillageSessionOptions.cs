/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：集中配置共享新手村会话、地图、区域和容量。
- 修改提示：生产环境应通过配置中心或环境变量覆盖，不在业务代码中写死新手村参数。
*/

namespace Game.Shared.Options;

public sealed class VillageSessionOptions
{
    public const string Section = "VillageSession";

    public string Mode { get; init; } = string.Empty;
    public string MapId { get; init; } = string.Empty;
    public string Region { get; init; } = string.Empty;
    public string? BuildVersion { get; init; }
    public int MaxPlayers { get; init; }
}
