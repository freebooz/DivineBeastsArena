namespace Game.Shared.Options;

/** 玩家区服目录的可观测缓存策略。目录修改后应主动失效，TTL 仅作为兜底。 */
public sealed class ServerDirectoryOptions
{
    public const string Section = "ServerDirectory";
    public int CacheTtlSeconds { get; init; } = 30;
}
