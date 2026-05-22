/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：测试代码，用于锁定关键业务契约，防止后续重构破坏已有行为。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.IntegrationTests;

public class GameDbContextModelTests
{
    [Fact]
    public async Task DbContext_CanPersistSessionAndPlayerSession()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"game-platform-{Guid.NewGuid()}")
            .Options;

        await using var db = new GameDbContext(options);
        var sessionId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "ROOM",
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Status = "CREATED",
            MaxPlayers = 8
        });
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            SessionTokenHash = "hash",
            SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(10)
        });

        await db.SaveChangesAsync();

        var loaded = await db.GameSessions
            .Include(x => x.PlayerSessions)
            .SingleAsync(x => x.Id == sessionId);

        Assert.Equal("CREATED", loaded.Status);
        Assert.Contains(loaded.PlayerSessions, x => x.PlayerId == playerId);
    }
}
