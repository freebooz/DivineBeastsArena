/*
中文阅读说明：
- 该迁移为历史玩家保留既有昵称，避免上线后覆盖任何已主动修改的玩家名称。
- 新账号由开户逻辑显式写入 false；客户端首次登录完成后，显式调用玩家名生成接口写入 3-5 个汉字游戏名。
- 下方迁移特性用于把该手写迁移注册到 EF Core 版本链，禁止删除。
*/

using Game.Infrastructure.Database;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations;

[DbContext(typeof(GameDbContext))]
[Migration("20260809140000_AddPlayerGameNameInitialization")]
public partial class AddPlayerGameNameInitialization : Migration
{
    protected override void Up(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.AddColumn<bool>(
            name: "game_name_initialized",
            table: "player_profile",
            type: "boolean",
            nullable: false,
            defaultValue: true);
    }

    protected override void Down(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.DropColumn(
            name: "game_name_initialized",
            table: "player_profile");
    }
}
