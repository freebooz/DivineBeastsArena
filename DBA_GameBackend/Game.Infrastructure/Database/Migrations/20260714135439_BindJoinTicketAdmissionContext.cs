using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations
{
    /// <inheritdoc />
    public partial class BindJoinTicketAdmissionContext : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<Guid>(
                name: "character_id",
                table: "player_session",
                type: "uuid",
                nullable: true);

            migrationBuilder.AddColumn<string>(
                name: "session_token_build_id",
                table: "player_session",
                type: "character varying(64)",
                maxLength: 64,
                nullable: true);

            migrationBuilder.AddColumn<Guid>(
                name: "session_token_server_id",
                table: "player_session",
                type: "uuid",
                nullable: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_session_character_id",
                table: "player_session",
                column: "character_id");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropIndex(
                name: "IX_player_session_character_id",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "character_id",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "session_token_build_id",
                table: "player_session");

            migrationBuilder.DropColumn(
                name: "session_token_server_id",
                table: "player_session");
        }
    }
}
