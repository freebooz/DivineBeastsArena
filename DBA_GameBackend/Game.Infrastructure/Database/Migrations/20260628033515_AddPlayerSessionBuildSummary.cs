using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations
{
    /// <inheritdoc />
    public partial class AddPlayerSessionBuildSummary : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<string>(
                name: "five_camp",
                table: "player_session",
                type: "character varying(32)",
                maxLength: 32,
                nullable: true);

            migrationBuilder.AddColumn<string>(
                name: "fixed_skill_group_id",
                table: "player_session",
                type: "character varying(64)",
                maxLength: 64,
                nullable: true);

            migrationBuilder.AddColumn<string>(
                name: "primary_element",
                table: "player_session",
                type: "character varying(32)",
                maxLength: 32,
                nullable: true);

            migrationBuilder.AddColumn<string>(
                name: "zodiac",
                table: "player_session",
                type: "character varying(32)",
                maxLength: 32,
                nullable: true);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "five_camp",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "fixed_skill_group_id",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "primary_element",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "zodiac",
                table: "player_session");
        }
    }
}
