/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Api.Services.Runtime;

public interface IRuntimeTokenService
{
    string GenerateToken(Guid serverId);
    bool ValidateToken(string token, out Guid serverId);
}

public sealed class RuntimeTokenService : IRuntimeTokenService
{
    private readonly Dictionary<string, Guid> _tokens = new();

    public string GenerateToken(Guid serverId)
    {
        var token = Guid.NewGuid().ToString();
        _tokens[token] = serverId;
        return token;
    }

    public bool ValidateToken(string token, out Guid serverId)
    {
        return _tokens.TryGetValue(token, out serverId!);
    }
}