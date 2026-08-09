using System;
using Game.Infrastructure.Database;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations;

[DbContext(typeof(GameDbContext))]
[Migration("20260809090000_AddGameServersDirectory")]
public partial class AddGameServersDirectory : Migration
{
    protected override void Up(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.CreateTable(
            name: "game_servers",
            columns: table => new
            {
                id = table.Column<Guid>(type: "uuid", nullable: false),
                name = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                platform = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                status = table.Column<string>(type: "character varying(16)", maxLength: 16, nullable: false),
                population = table.Column<int>(type: "integer", nullable: false),
                recommended = table.Column<bool>(type: "boolean", nullable: false),
                maintenance_message = table.Column<string>(type: "character varying(512)", maxLength: 512, nullable: true),
                min_client_version = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
            },
            constraints: table => table.PrimaryKey("PK_game_servers", x => x.id));

        migrationBuilder.CreateIndex(
            name: "IX_game_servers_region_platform_status",
            table: "game_servers",
            columns: new[] { "region", "platform", "status" });

        migrationBuilder.CreateIndex(
            name: "IX_game_servers_recommended_population",
            table: "game_servers",
            columns: new[] { "recommended", "population" });
    }

    protected override void Down(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.DropTable(name: "game_servers");
    }
}
