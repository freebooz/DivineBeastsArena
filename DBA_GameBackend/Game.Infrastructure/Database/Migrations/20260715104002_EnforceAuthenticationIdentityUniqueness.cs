using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations
{
    /// <inheritdoc />
    public partial class EnforceAuthenticationIdentityUniqueness : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.Sql(
                """
                DO $$
                BEGIN
                    IF EXISTS (
                        SELECT 1
                        FROM player_identity
                        GROUP BY display_name
                        HAVING COUNT(*) > 1
                    ) THEN
                        RAISE EXCEPTION '检测到重复玩家显示名，请人工审查并处理后重新执行迁移。';
                    END IF;

                    IF EXISTS (
                        SELECT 1
                        FROM device_login
                        GROUP BY device_id_hash
                        HAVING COUNT(*) > 1
                    ) THEN
                        RAISE EXCEPTION '检测到重复设备身份哈希，请人工审查并处理后重新执行迁移。';
                    END IF;
                END
                $$;
                """);

            migrationBuilder.DropIndex(
                name: "IX_player_identity_display_name",
                table: "player_identity");

            migrationBuilder.DropIndex(
                name: "IX_device_login_account_id_device_id_hash",
                table: "device_login");

            migrationBuilder.CreateIndex(
                name: "IX_player_identity_display_name",
                table: "player_identity",
                column: "display_name",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_device_login_account_id",
                table: "device_login",
                column: "account_id");

            migrationBuilder.CreateIndex(
                name: "IX_device_login_device_id_hash",
                table: "device_login",
                column: "device_id_hash",
                unique: true);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropIndex(
                name: "IX_player_identity_display_name",
                table: "player_identity");

            migrationBuilder.DropIndex(
                name: "IX_device_login_account_id",
                table: "device_login");

            migrationBuilder.DropIndex(
                name: "IX_device_login_device_id_hash",
                table: "device_login");

            migrationBuilder.CreateIndex(
                name: "IX_player_identity_display_name",
                table: "player_identity",
                column: "display_name");

            migrationBuilder.CreateIndex(
                name: "IX_device_login_account_id_device_id_hash",
                table: "device_login",
                columns: new[] { "account_id", "device_id_hash" },
                unique: true);
        }
    }
}
