/*
中文阅读说明：
- 该迁移为历史玩家保留既有昵称，避免上线后覆盖任何已主动修改的玩家名称。
- 新账号由开户逻辑显式写入 false，并在首次登录的事务性补偿流程中生成 3-5 个汉字游戏名。
*/

using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations;

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
