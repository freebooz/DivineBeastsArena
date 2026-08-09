using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations;

public partial class AddCharacterServiceAuthority : Migration
{
    protected override void Up(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.RenameTable(name: "player_character", newName: "characters");
        migrationBuilder.AddColumn<Guid>(name: "server_id", table: "characters", nullable: false, defaultValue: Guid.Empty);
        migrationBuilder.AddColumn<string>(name: "normalized_name", table: "characters", maxLength: 64, nullable: false, defaultValue: string.Empty);
        migrationBuilder.AddColumn<bool>(name: "is_deleted", table: "characters", nullable: false, defaultValue: false);
        migrationBuilder.AddColumn<DateTimeOffset>(name: "deleted_at", table: "characters", nullable: true);
        migrationBuilder.AddColumn<string>(name: "creation_idempotency_key", table: "characters", maxLength: 128, nullable: true);
        migrationBuilder.Sql("UPDATE characters SET normalized_name = UPPER(character_name) WHERE normalized_name = '';");

        migrationBuilder.DropIndex(name: "IX_player_character_player_id_character_name", table: "characters");
        migrationBuilder.DropIndex(name: "IX_player_character_player_id_is_selected", table: "characters");
        migrationBuilder.DropIndex(name: "IX_player_character_player_id", table: "characters");
        migrationBuilder.CreateIndex(name: "IX_characters_player_id_server_id_is_deleted", table: "characters", columns: new[] { "player_id", "server_id", "is_deleted" });
        migrationBuilder.CreateIndex(name: "IX_characters_server_id_normalized_name", table: "characters", columns: new[] { "server_id", "normalized_name" }, unique: true);
        migrationBuilder.CreateIndex(name: "IX_characters_player_id_server_id_is_selected", table: "characters", columns: new[] { "player_id", "server_id", "is_selected" });
        migrationBuilder.CreateIndex(name: "IX_characters_player_id_server_id_creation_idempotency_key", table: "characters", columns: new[] { "player_id", "server_id", "creation_idempotency_key" }, unique: true);

        migrationBuilder.CreateTable(
            name: "character_appearances",
            columns: table => new
            {
                character_id = table.Column<Guid>(nullable: false),
                rules_version = table.Column<string>(maxLength: 32, nullable: false),
                appearance_json = table.Column<string>(type: "jsonb", nullable: false),
                created_at = table.Column<DateTimeOffset>(nullable: false),
                updated_at = table.Column<DateTimeOffset>(nullable: false)
            },
            constraints: table =>
            {
                table.PrimaryKey("PK_character_appearances", x => x.character_id);
                table.ForeignKey("FK_character_appearances_characters_character_id", x => x.character_id, "characters", "id", onDelete: ReferentialAction.Cascade);
            });
        migrationBuilder.CreateTable(
            name: "character_progress",
            columns: table => new
            {
                character_id = table.Column<Guid>(nullable: false),
                level = table.Column<int>(nullable: false),
                experience = table.Column<long>(nullable: false),
                updated_at = table.Column<DateTimeOffset>(nullable: false)
            },
            constraints: table =>
            {
                table.PrimaryKey("PK_character_progress", x => x.character_id);
                table.ForeignKey("FK_character_progress_characters_character_id", x => x.character_id, "characters", "id", onDelete: ReferentialAction.Cascade);
            });
    }

    protected override void Down(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.DropTable(name: "character_appearances");
        migrationBuilder.DropTable(name: "character_progress");
        migrationBuilder.DropIndex(name: "IX_characters_player_id_server_id_is_deleted", table: "characters");
        migrationBuilder.DropIndex(name: "IX_characters_server_id_normalized_name", table: "characters");
        migrationBuilder.DropIndex(name: "IX_characters_player_id_server_id_is_selected", table: "characters");
        migrationBuilder.DropIndex(name: "IX_characters_player_id_server_id_creation_idempotency_key", table: "characters");
        migrationBuilder.DropColumn(name: "server_id", table: "characters");
        migrationBuilder.DropColumn(name: "normalized_name", table: "characters");
        migrationBuilder.DropColumn(name: "is_deleted", table: "characters");
        migrationBuilder.DropColumn(name: "deleted_at", table: "characters");
        migrationBuilder.DropColumn(name: "creation_idempotency_key", table: "characters");
        migrationBuilder.RenameTable(name: "characters", newName: "player_character");
        migrationBuilder.CreateIndex(name: "IX_player_character_player_id", table: "player_character", column: "player_id");
        migrationBuilder.CreateIndex(name: "IX_player_character_player_id_character_name", table: "player_character", columns: new[] { "player_id", "character_name" }, unique: true);
        migrationBuilder.CreateIndex(name: "IX_player_character_player_id_is_selected", table: "player_character", columns: new[] { "player_id", "is_selected" });
    }
}
