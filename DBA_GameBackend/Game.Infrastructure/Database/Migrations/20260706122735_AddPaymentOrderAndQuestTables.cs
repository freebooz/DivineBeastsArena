using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Game.Infrastructure.Database.Migrations
{
    /// <inheritdoc />
    public partial class AddPaymentOrderAndQuestTables : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "payment_order",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    platform = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    platform_order_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    amount = table.Column<long>(type: "bigint", nullable: false),
                    currency = table.Column<string>(type: "character varying(16)", maxLength: 16, nullable: false),
                    product_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    product_name = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    virtual_amount = table.Column<long>(type: "bigint", nullable: false),
                    virtual_currency = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    callback_json = table.Column<string>(type: "jsonb", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    paid_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_payment_order", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "quest",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    quest_key = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    title = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: false),
                    description = table.Column<string>(type: "text", nullable: false),
                    quest_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    category = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    target_progress = table.Column<int>(type: "integer", nullable: false),
                    reward_json = table.Column<string>(type: "jsonb", nullable: false),
                    sort_order = table.Column<int>(type: "integer", nullable: false),
                    is_active = table.Column<bool>(type: "boolean", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_quest", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "player_quest",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    quest_id = table.Column<Guid>(type: "uuid", nullable: false),
                    progress = table.Column<int>(type: "integer", nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    accepted_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    completed_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    rewarded_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    expired_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_quest", x => x.id);
                    table.ForeignKey(
                        name: "FK_player_quest_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                    table.ForeignKey(
                        name: "FK_player_quest_quest_quest_id",
                        column: x => x.quest_id,
                        principalTable: "quest",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateIndex(
                name: "IX_payment_order_platform_order_id",
                table: "payment_order",
                column: "platform_order_id");

            migrationBuilder.CreateIndex(
                name: "IX_payment_order_player_id",
                table: "payment_order",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_payment_order_status",
                table: "payment_order",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_player_quest_player_id",
                table: "player_quest",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_player_quest_player_id_quest_id",
                table: "player_quest",
                columns: new[] { "player_id", "quest_id" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_quest_quest_id",
                table: "player_quest",
                column: "quest_id");

            migrationBuilder.CreateIndex(
                name: "IX_player_quest_status",
                table: "player_quest",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_quest_is_active",
                table: "quest",
                column: "is_active");

            migrationBuilder.CreateIndex(
                name: "IX_quest_quest_key",
                table: "quest",
                column: "quest_key",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_quest_quest_type",
                table: "quest",
                column: "quest_type");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "payment_order");

            migrationBuilder.DropTable(
                name: "player_quest");

            migrationBuilder.DropTable(
                name: "quest");
        }
    }
}
