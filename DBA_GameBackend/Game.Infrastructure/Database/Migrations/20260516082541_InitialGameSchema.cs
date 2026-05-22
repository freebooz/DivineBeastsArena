/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System;
using Microsoft.EntityFrameworkCore.Migrations;
using Npgsql.EntityFrameworkCore.PostgreSQL.Metadata;

#nullable disable

namespace Game.Infrastructure.Database.Migrations
{
    /// <inheritdoc />
    public partial class InitialGameSchema : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "account",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    account_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    email = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: true),
                    steam_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    eos_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    last_login_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_account", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "admin_audit_log",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    admin_user_id = table.Column<Guid>(type: "uuid", nullable: true),
                    action = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    target_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    target_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    reason = table.Column<string>(type: "text", nullable: true),
                    before_json = table.Column<string>(type: "jsonb", nullable: true),
                    after_json = table.Column<string>(type: "jsonb", nullable: true),
                    ip_address = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    user_agent = table.Column<string>(type: "text", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_admin_audit_log", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "admin_user",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    username = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    password_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: false),
                    role = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    failed_login_count = table.Column<int>(type: "integer", nullable: false),
                    locked_until = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    last_login_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_admin_user", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "crash_report",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: true),
                    client_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    platform = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    crash_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    title = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: true),
                    description = table.Column<string>(type: "text", nullable: true),
                    dump_url = table.Column<string>(type: "text", nullable: true),
                    log_url = table.Column<string>(type: "text", nullable: true),
                    metadata_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_crash_report", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "game_config",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    config_key = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    content_json = table.Column<string>(type: "jsonb", nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    checksum = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    channel = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    min_client_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    max_client_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    created_by = table.Column<Guid>(type: "uuid", nullable: true),
                    published_by = table.Column<Guid>(type: "uuid", nullable: true),
                    published_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_config", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "game_config_publish_log",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    config_key = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    from_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    to_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    operator_id = table.Column<Guid>(type: "uuid", nullable: true),
                    reason = table.Column<string>(type: "text", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_config_publish_log", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "game_room",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    owner_player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    mode = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    map_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    max_players = table.Column<int>(type: "integer", nullable: false),
                    visibility = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    password_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: true),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    closed_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_room", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "game_server_instance",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    session_id = table.Column<Guid>(type: "uuid", nullable: true),
                    mode = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    map_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    build_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    ip = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    port = table.Column<int>(type: "integer", nullable: false),
                    process_id = table.Column<int>(type: "integer", nullable: true),
                    container_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    runtime_token_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: true),
                    runtime_token_expires_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    started_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    ready_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    allocated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    ended_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    last_heartbeat_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    exit_code = table.Column<int>(type: "integer", nullable: true),
                    crash_reason = table.Column<string>(type: "text", nullable: true),
                    log_path = table.Column<string>(type: "text", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_server_instance", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "game_session",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    source_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    source_id = table.Column<Guid>(type: "uuid", nullable: true),
                    mode = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    map_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    server_id = table.Column<Guid>(type: "uuid", nullable: true),
                    server_ip = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    server_port = table.Column<int>(type: "integer", nullable: true),
                    build_version = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    max_players = table.Column<int>(type: "integer", nullable: false),
                    retry_count = table.Column<int>(type: "integer", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    allocated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    started_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    ended_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_session", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "inventory_log",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    item_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    quantity_delta = table.Column<long>(type: "bigint", nullable: false),
                    quantity_before = table.Column<long>(type: "bigint", nullable: false),
                    quantity_after = table.Column<long>(type: "bigint", nullable: false),
                    reason = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    biz_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    biz_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    operator_id = table.Column<Guid>(type: "uuid", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_inventory_log", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "matchmaking_ticket",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    mode = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    region = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    mmr = table.Column<int>(type: "integer", nullable: false, defaultValue: 1000),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    matched_session_id = table.Column<Guid>(type: "uuid", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    cancelled_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    timeout_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_matchmaking_ticket", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "order_record",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    platform = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    platform_order_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    amount = table.Column<long>(type: "bigint", nullable: false),
                    currency = table.Column<string>(type: "character varying(16)", maxLength: 16, nullable: false),
                    item_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    paid_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    completed_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_order_record", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "player_event_log",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    event_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    payload_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_event_log", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "player_feedback",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: true),
                    nickname = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    email = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: true),
                    feedback_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    title = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: true),
                    content = table.Column<string>(type: "text", nullable: false),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    handled_by = table.Column<Guid>(type: "uuid", nullable: true),
                    handled_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    handle_note = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_feedback", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "player_profile",
                columns: table => new
                {
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    nickname = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    avatar = table.Column<string>(type: "character varying(255)", maxLength: 255, nullable: true),
                    level = table.Column<int>(type: "integer", nullable: false, defaultValue: 1),
                    exp = table.Column<long>(type: "bigint", nullable: false, defaultValue: 0L),
                    nickname_updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    last_login_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_profile", x => x.player_id);
                });

            migrationBuilder.CreateTable(
                name: "port_allocation",
                columns: table => new
                {
                    port = table.Column<int>(type: "integer", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    server_id = table.Column<Guid>(type: "uuid", nullable: true),
                    allocated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    released_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_port_allocation", x => x.port);
                });

            migrationBuilder.CreateTable(
                name: "wallet_balance",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    currency_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    balance = table.Column<long>(type: "bigint", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_wallet_balance", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "wallet_ledger",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    currency_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    amount = table.Column<long>(type: "bigint", nullable: false),
                    balance_before = table.Column<long>(type: "bigint", nullable: false),
                    balance_after = table.Column<long>(type: "bigint", nullable: false),
                    biz_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    biz_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    idempotency_key = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    operator_id = table.Column<Guid>(type: "uuid", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_wallet_ledger", x => x.id);
                });

            migrationBuilder.CreateTable(
                name: "ban_record",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    account_id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: true),
                    reason = table.Column<string>(type: "text", nullable: false),
                    starts_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    ends_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_by = table.Column<Guid>(type: "uuid", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    revoked_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    revoked_by = table.Column<Guid>(type: "uuid", nullable: true),
                    revoke_reason = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_ban_record", x => x.id);
                    table.ForeignKey(
                        name: "FK_ban_record_account_account_id",
                        column: x => x.account_id,
                        principalTable: "account",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "device_login",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    account_id = table.Column<Guid>(type: "uuid", nullable: false),
                    device_id_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: false),
                    device_name = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: true),
                    platform = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    last_login_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_device_login", x => x.id);
                    table.ForeignKey(
                        name: "FK_device_login_account_account_id",
                        column: x => x.account_id,
                        principalTable: "account",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "refresh_token",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    account_id = table.Column<Guid>(type: "uuid", nullable: false),
                    token_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: false),
                    expires_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    revoked_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    created_by_ip = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: true),
                    user_agent = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_refresh_token", x => x.id);
                    table.ForeignKey(
                        name: "FK_refresh_token_account_account_id",
                        column: x => x.account_id,
                        principalTable: "account",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "game_room_player",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    room_id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    slot_index = table.Column<int>(type: "integer", nullable: false),
                    team = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    is_ready = table.Column<bool>(type: "boolean", nullable: false),
                    joined_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    left_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_room_player", x => x.id);
                    table.ForeignKey(
                        name: "FK_game_room_player_game_room_room_id",
                        column: x => x.room_id,
                        principalTable: "game_room",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "game_server_event",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    server_id = table.Column<Guid>(type: "uuid", nullable: false),
                    event_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    payload_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_game_server_event", x => x.id);
                    table.ForeignKey(
                        name: "FK_game_server_event_game_server_instance_server_id",
                        column: x => x.server_id,
                        principalTable: "game_server_instance",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "match_result",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    session_id = table.Column<Guid>(type: "uuid", nullable: false),
                    server_id = table.Column<Guid>(type: "uuid", nullable: false),
                    mode = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    map_id = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    duration_seconds = table.Column<int>(type: "integer", nullable: false),
                    result_json = table.Column<string>(type: "jsonb", nullable: false),
                    idempotency_key = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    GameSessionId = table.Column<Guid>(type: "uuid", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_match_result", x => x.id);
                    table.ForeignKey(
                        name: "FK_match_result_game_session_GameSessionId",
                        column: x => x.GameSessionId,
                        principalTable: "game_session",
                        principalColumn: "id");
                });

            migrationBuilder.CreateTable(
                name: "player_session",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    game_session_id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    team = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    slot_index = table.Column<int>(type: "integer", nullable: true),
                    status = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    session_token_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: false),
                    session_token_expires_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    reconnect_token_hash = table.Column<string>(type: "character varying(256)", maxLength: 256, nullable: true),
                    reconnect_token_expires_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    joined_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    left_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_session", x => x.id);
                    table.ForeignKey(
                        name: "FK_player_session_game_session_game_session_id",
                        column: x => x.game_session_id,
                        principalTable: "game_session",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "session_event",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    game_session_id = table.Column<Guid>(type: "uuid", nullable: false),
                    event_type = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    payload_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_session_event", x => x.id);
                    table.ForeignKey(
                        name: "FK_session_event_game_session_game_session_id",
                        column: x => x.game_session_id,
                        principalTable: "game_session",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "inventory_item",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    item_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    quantity = table.Column<long>(type: "bigint", nullable: false),
                    expires_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: true),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_inventory_item", x => x.id);
                    table.ForeignKey(
                        name: "FK_inventory_item_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "player_identity",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    account_id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    display_name = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_identity", x => x.id);
                    table.ForeignKey(
                        name: "FK_player_identity_account_account_id",
                        column: x => x.account_id,
                        principalTable: "account",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                    table.ForeignKey(
                        name: "FK_player_identity_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "player_settings",
                columns: table => new
                {
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    settings_json = table.Column<string>(type: "jsonb", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_settings", x => x.player_id);
                    table.ForeignKey(
                        name: "FK_player_settings_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "player_statistics",
                columns: table => new
                {
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    total_matches = table.Column<int>(type: "integer", nullable: false),
                    wins = table.Column<int>(type: "integer", nullable: false),
                    losses = table.Column<int>(type: "integer", nullable: false),
                    draws = table.Column<int>(type: "integer", nullable: false),
                    kills = table.Column<int>(type: "integer", nullable: false),
                    deaths = table.Column<int>(type: "integer", nullable: false),
                    assists = table.Column<int>(type: "integer", nullable: false),
                    score = table.Column<long>(type: "bigint", nullable: false),
                    play_time_seconds = table.Column<long>(type: "bigint", nullable: false),
                    updated_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_statistics", x => x.player_id);
                    table.ForeignKey(
                        name: "FK_player_statistics_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "player_unlock",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    unlock_type = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    unlock_id = table.Column<string>(type: "character varying(128)", maxLength: 128, nullable: false),
                    source = table.Column<string>(type: "character varying(64)", maxLength: 64, nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_player_unlock", x => x.id);
                    table.ForeignKey(
                        name: "FK_player_unlock_player_profile_player_id",
                        column: x => x.player_id,
                        principalTable: "player_profile",
                        principalColumn: "player_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "match_player_result",
                columns: table => new
                {
                    id = table.Column<Guid>(type: "uuid", nullable: false),
                    match_result_id = table.Column<Guid>(type: "uuid", nullable: false),
                    player_id = table.Column<Guid>(type: "uuid", nullable: false),
                    team = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: true),
                    result = table.Column<string>(type: "character varying(32)", maxLength: 32, nullable: false),
                    kills = table.Column<int>(type: "integer", nullable: false),
                    deaths = table.Column<int>(type: "integer", nullable: false),
                    assists = table.Column<int>(type: "integer", nullable: false),
                    score = table.Column<int>(type: "integer", nullable: false),
                    exp_delta = table.Column<long>(type: "bigint", nullable: false),
                    reward_json = table.Column<string>(type: "jsonb", nullable: false),
                    created_at = table.Column<DateTimeOffset>(type: "timestamp with time zone", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_match_player_result", x => x.id);
                    table.ForeignKey(
                        name: "FK_match_player_result_match_result_match_result_id",
                        column: x => x.match_result_id,
                        principalTable: "match_result",
                        principalColumn: "id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateIndex(
                name: "IX_account_created_at",
                table: "account",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_account_email",
                table: "account",
                column: "email",
                unique: true,
                filter: "email IS NOT NULL");

            migrationBuilder.CreateIndex(
                name: "IX_account_eos_id",
                table: "account",
                column: "eos_id",
                unique: true,
                filter: "eos_id IS NOT NULL");

            migrationBuilder.CreateIndex(
                name: "IX_account_status",
                table: "account",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_account_steam_id",
                table: "account",
                column: "steam_id",
                unique: true,
                filter: "steam_id IS NOT NULL");

            migrationBuilder.CreateIndex(
                name: "IX_admin_audit_log_action",
                table: "admin_audit_log",
                column: "action");

            migrationBuilder.CreateIndex(
                name: "IX_admin_audit_log_admin_user_id",
                table: "admin_audit_log",
                column: "admin_user_id");

            migrationBuilder.CreateIndex(
                name: "IX_admin_audit_log_created_at",
                table: "admin_audit_log",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_admin_user_username",
                table: "admin_user",
                column: "username",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_ban_record_account_id",
                table: "ban_record",
                column: "account_id");

            migrationBuilder.CreateIndex(
                name: "IX_ban_record_player_id",
                table: "ban_record",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_device_login_account_id_device_id_hash",
                table: "device_login",
                columns: new[] { "account_id", "device_id_hash" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_game_config_config_key",
                table: "game_config",
                column: "config_key");

            migrationBuilder.CreateIndex(
                name: "IX_game_config_config_key_version_channel_region",
                table: "game_config",
                columns: new[] { "config_key", "version", "channel", "region" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_game_config_status",
                table: "game_config",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_game_config_publish_log_config_key",
                table: "game_config_publish_log",
                column: "config_key");

            migrationBuilder.CreateIndex(
                name: "IX_game_config_publish_log_created_at",
                table: "game_config_publish_log",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_game_room_created_at",
                table: "game_room",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_game_room_mode_region",
                table: "game_room",
                columns: new[] { "mode", "region" });

            migrationBuilder.CreateIndex(
                name: "IX_game_room_status",
                table: "game_room",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_game_room_player_room_id_player_id",
                table: "game_room_player",
                columns: new[] { "room_id", "player_id" },
                filter: "left_at IS NULL");

            migrationBuilder.CreateIndex(
                name: "IX_game_server_event_server_id",
                table: "game_server_event",
                column: "server_id");

            migrationBuilder.CreateIndex(
                name: "IX_game_server_instance_last_heartbeat_at",
                table: "game_server_instance",
                column: "last_heartbeat_at");

            migrationBuilder.CreateIndex(
                name: "IX_game_server_instance_session_id",
                table: "game_server_instance",
                column: "session_id");

            migrationBuilder.CreateIndex(
                name: "IX_game_server_instance_status",
                table: "game_server_instance",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_game_session_created_at",
                table: "game_session",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_game_session_mode_region",
                table: "game_session",
                columns: new[] { "mode", "region" });

            migrationBuilder.CreateIndex(
                name: "IX_game_session_status",
                table: "game_session",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_inventory_item_player_id",
                table: "inventory_item",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_inventory_log_biz_type",
                table: "inventory_log",
                column: "biz_type");

            migrationBuilder.CreateIndex(
                name: "IX_inventory_log_player_id",
                table: "inventory_log",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_match_player_result_match_result_id_player_id",
                table: "match_player_result",
                columns: new[] { "match_result_id", "player_id" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_match_result_GameSessionId",
                table: "match_result",
                column: "GameSessionId");

            migrationBuilder.CreateIndex(
                name: "IX_match_result_idempotency_key",
                table: "match_result",
                column: "idempotency_key",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_match_result_session_id",
                table: "match_result",
                column: "session_id",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_matchmaking_ticket_mode_region_status",
                table: "matchmaking_ticket",
                columns: new[] { "mode", "region", "status" });

            migrationBuilder.CreateIndex(
                name: "IX_matchmaking_ticket_player_id",
                table: "matchmaking_ticket",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_matchmaking_ticket_status",
                table: "matchmaking_ticket",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_order_record_player_id",
                table: "order_record",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_order_record_status",
                table: "order_record",
                column: "status");

            migrationBuilder.CreateIndex(
                name: "IX_player_event_log_created_at",
                table: "player_event_log",
                column: "created_at");

            migrationBuilder.CreateIndex(
                name: "IX_player_event_log_event_type",
                table: "player_event_log",
                column: "event_type");

            migrationBuilder.CreateIndex(
                name: "IX_player_event_log_player_id",
                table: "player_event_log",
                column: "player_id");

            migrationBuilder.CreateIndex(
                name: "IX_player_identity_account_id",
                table: "player_identity",
                column: "account_id",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_identity_display_name",
                table: "player_identity",
                column: "display_name");

            migrationBuilder.CreateIndex(
                name: "IX_player_identity_player_id",
                table: "player_identity",
                column: "player_id",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_profile_last_login_at",
                table: "player_profile",
                column: "last_login_at");

            migrationBuilder.CreateIndex(
                name: "IX_player_profile_level",
                table: "player_profile",
                column: "level");

            migrationBuilder.CreateIndex(
                name: "IX_player_profile_nickname",
                table: "player_profile",
                column: "nickname",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_session_game_session_id_player_id",
                table: "player_session",
                columns: new[] { "game_session_id", "player_id" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_session_session_token_hash",
                table: "player_session",
                column: "session_token_hash",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_player_unlock_player_id_unlock_type_unlock_id",
                table: "player_unlock",
                columns: new[] { "player_id", "unlock_type", "unlock_id" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_refresh_token_account_id",
                table: "refresh_token",
                column: "account_id");

            migrationBuilder.CreateIndex(
                name: "IX_refresh_token_expires_at",
                table: "refresh_token",
                column: "expires_at");

            migrationBuilder.CreateIndex(
                name: "IX_refresh_token_token_hash",
                table: "refresh_token",
                column: "token_hash",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_session_event_game_session_id",
                table: "session_event",
                column: "game_session_id");

            migrationBuilder.CreateIndex(
                name: "IX_wallet_balance_player_id_currency_type",
                table: "wallet_balance",
                columns: new[] { "player_id", "currency_type" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_wallet_ledger_idempotency_key",
                table: "wallet_ledger",
                column: "idempotency_key",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_wallet_ledger_player_id",
                table: "wallet_ledger",
                column: "player_id");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "admin_audit_log");

            migrationBuilder.DropTable(
                name: "admin_user");

            migrationBuilder.DropTable(
                name: "ban_record");

            migrationBuilder.DropTable(
                name: "crash_report");

            migrationBuilder.DropTable(
                name: "device_login");

            migrationBuilder.DropTable(
                name: "game_config");

            migrationBuilder.DropTable(
                name: "game_config_publish_log");

            migrationBuilder.DropTable(
                name: "game_room_player");

            migrationBuilder.DropTable(
                name: "game_server_event");

            migrationBuilder.DropTable(
                name: "inventory_item");

            migrationBuilder.DropTable(
                name: "inventory_log");

            migrationBuilder.DropTable(
                name: "match_player_result");

            migrationBuilder.DropTable(
                name: "matchmaking_ticket");

            migrationBuilder.DropTable(
                name: "order_record");

            migrationBuilder.DropTable(
                name: "player_event_log");

            migrationBuilder.DropTable(
                name: "player_feedback");

            migrationBuilder.DropTable(
                name: "player_identity");

            migrationBuilder.DropTable(
                name: "player_session");

            migrationBuilder.DropTable(
                name: "player_settings");

            migrationBuilder.DropTable(
                name: "player_statistics");

            migrationBuilder.DropTable(
                name: "player_unlock");

            migrationBuilder.DropTable(
                name: "port_allocation");

            migrationBuilder.DropTable(
                name: "refresh_token");

            migrationBuilder.DropTable(
                name: "session_event");

            migrationBuilder.DropTable(
                name: "wallet_balance");

            migrationBuilder.DropTable(
                name: "wallet_ledger");

            migrationBuilder.DropTable(
                name: "game_room");

            migrationBuilder.DropTable(
                name: "game_server_instance");

            migrationBuilder.DropTable(
                name: "match_result");

            migrationBuilder.DropTable(
                name: "player_profile");

            migrationBuilder.DropTable(
                name: "account");

            migrationBuilder.DropTable(
                name: "game_session");
        }
    }
}
