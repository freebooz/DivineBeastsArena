/*
中文阅读说明：
- 本配置是服务端自动生成游戏玩家名的唯一数据入口；姓名字符库属于可运营数据，不能散落在业务代码中。
- 账号登录名、PlayerIdentity.DisplayName 与游戏玩家名是不同概念：本配置只作用于 PlayerProfile.Nickname。
*/

using System;

namespace Game.Shared.Options;

public sealed class PlayerGameNameOptions
{
    public const string Section = "PlayerGameName";

    /** 生成名称的最小汉字数；产品要求为 3。 */
    public int MinimumHanCharacters { get; init; } = 3;

    /** 生成名称的最大汉字数；产品要求为 5。 */
    public int MaximumHanCharacters { get; init; } = 5;

    /** 单次首次登录最多重试次数，用于处理昵称唯一索引并发冲突。 */
    public int GenerationAttempts { get; init; } = 32;

    /** 单字姓氏库，由运营配置维护。 */
    public string[] Surnames { get; init; } = Array.Empty<string>();

    /** 单个名字用汉字库，由运营配置维护。 */
    public string[] GivenNameCharacters { get; init; } = Array.Empty<string>();
}
