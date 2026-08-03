using Game.Infrastructure.Database;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations;

[DbContext(typeof(GameDbContext))]
[Migration("20260715090000_AddSessionSourceUniqueness")]
public partial class AddSessionSourceUniqueness : Migration
{
    protected override void Up(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.Sql("""
            DO $$
            BEGIN
                IF EXISTS (
                    SELECT 1
                    FROM game_session
                    WHERE source_id IS NOT NULL
                    GROUP BY source_type, source_id
                    HAVING COUNT(*) > 1
                ) THEN
                    RAISE EXCEPTION '检测到重复的会话来源，请先人工审查重复会话后再应用唯一索引。';
                END IF;
            END $$;
            """);

        migrationBuilder.CreateIndex(
            name: "IX_game_session_source_type_source_id",
            table: "game_session",
            columns: new[] { "source_type", "source_id" },
            unique: true,
            filter: "source_id IS NOT NULL");
    }

    protected override void Down(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.DropIndex(
            name: "IX_game_session_source_type_source_id",
            table: "game_session");
    }
}
