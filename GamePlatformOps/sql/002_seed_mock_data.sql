-- 002_seed_mock_data.sql
-- Seed mock data for development and testing
-- Execution: AFTER 001_create_schema.sql has been applied

-- ============================================================================
-- ADMIN USERS
-- ============================================================================

INSERT INTO admin_users (id, username, password_hash, role, status) VALUES
    ('a0000000-0000-0000-0000-000000000001', 'admin', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'SUPER_ADMIN', 'ACTIVE'),
    ('a0000000-0000-0000-0000-000000000002', 'gm_alpha', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'GM', 'ACTIVE'),
    ('a0000000-0000-0000-0000-000000000003', 'gm_beta', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'GM', 'ACTIVE')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME CONFIGS
-- ============================================================================

INSERT INTO game_configs (id, config_key, config_type, config_value, version, status, checksum, created_by, published_at) VALUES
    ('c0000000-0000-0000-0000-000000000001', 'matchmaking_config', 'MATCHMAKING', '{"queue_timeout_seconds": 300, "min_mmr_delta": 100, "max_mmr_delta": 500, "team_size": 5}', 1, 'PUBLISHED', 'abc123', 'a0000000-0000-0000-0000-000000000001', NOW()),
    ('c0000000-0000-0000-0000-000000000002', 'ranked_season_1', 'RANKED', '{"season_id": "season_1", "start_date": "2026-01-01", "end_date": "2026-06-30", "mmr_thresholds": {"bronze": 1000, "silver": 1200, "gold": 1400, "platinum": 1600}}', 1, 'PUBLISHED', 'def456', 'a0000000-0000-0000-0000-000000000001', NOW()),
    ('c0000000-0000-0000-0000-000000000003', 'server_regions', 'SERVER', '{"regions": ["NA_EAST", "NA_WEST", "EU_WEST", "EU_EAST", "ASIA_PACIFIC"], "default_region": "NA_EAST"}', 1, 'PUBLISHED', 'ghi789', 'a0000000-0000-0000-0000-000000000001', NOW()),
    ('c0000000-0000-0000-0000-000000000004', 'item_shop_config', 'ECONOMY', '{"daily_item_ids": ["skin_001", "skin_002", "emote_001"], "featured_item_ids": ["legendary_skin_001"], "refresh_interval_hours": 24}', 1, 'DRAFT', NULL, NULL, NULL)
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- ACCOUNTS
-- ============================================================================

INSERT INTO accounts (id, email, password_hash, status, failed_login_count, locked_until) VALUES
    ('b0000000-0000-0000-0000-000000000001', 'player1@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000002', 'player2@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000003', 'player3@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000004', 'player4@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000005', 'player5@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000006', 'banned@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'BANNED', 0, NULL),
    ('b0000000-0000-0000-0000-000000000007', 'locked@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'LOCKED', 5, NOW() + INTERVAL '30 minutes'),
    ('b0000000-0000-0000-0000-000000000008', 'player8@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000009', 'player9@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL),
    ('b0000000-0000-0000-0000-000000000010', 'player10@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0, NULL)
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PLAYER IDENTITIES
-- ============================================================================

INSERT INTO player_identities (id, account_id, player_id, display_name) VALUES
    ('p0000000-0000-0000-0000-000000000001', 'b0000000-0000-0000-0000-000000000001', '11111111-1111-1111-1111-111111111111', 'NeonStriker'),
    ('p0000000-0000-0000-0000-000000000002', 'b0000000-0000-0000-0000-000000000002', '22222222-2222-2222-2222-222222222222', 'ShadowBlade'),
    ('p0000000-0000-0000-0000-000000000003', 'b0000000-0000-0000-0000-000000000003', '33333333-3333-3333-3333-333333333333', 'CyberPhoenix'),
    ('p0000000-0000-0000-0000-000000000004', 'b0000000-0000-0000-0000-000000000004', '44444444-4444-4444-4444-444444444444', 'QuantumRacer'),
    ('p0000000-0000-0000-0000-000000000005', 'b0000000-0000-0000-0000-000000000005', '55555555-5555-5555-5555-555555555555', 'ThunderWolf'),
    ('p0000000-0000-0000-0000-000000000006', 'b0000000-0000-0000-0000-000000000006', '66666666-6666-6666-6666-666666666666', 'VoidWalker'),
    ('p0000000-0000-0000-0000-000000000007', 'b0000000-0000-0000-0000-000000000007', '77777777-7777-7777-7777-777777777777', 'FrostByte'),
    ('p0000000-0000-0000-0000-000000000008', 'b0000000-0000-0000-0000-000000000008', '88888888-8888-8888-8888-888888888888', 'IronTitan'),
    ('p0000000-0000-0000-0000-000000000009', 'b0000000-0000-0000-0000-000000000009', '99999999-9999-9999-9999-999999999999', 'BlazeFury'),
    ('p0000000-0000-0000-0000-000000000010', 'b0000000-0000-0000-0000-000000000010', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'SpectreGhost')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PLAYER PROFILES
-- ============================================================================

INSERT INTO player_profiles (player_id, nickname, level, exp, status, avatar_url) VALUES
    ('11111111-1111-1111-1111-111111111111', 'NeonStriker', 42, 154200, 'ONLINE', 'https://cdn.example.com/avatars/neon.png'),
    ('22222222-2222-2222-2222-222222222222', 'ShadowBlade', 38, 128500, 'IN_MATCH', 'https://cdn.example.com/avatars/shadow.png'),
    ('33333333-3333-3333-3333-333333333333', 'CyberPhoenix', 55, 210000, 'ONLINE', 'https://cdn.example.com/avatars/phoenix.png'),
    ('44444444-4444-4444-4444-444444444444', 'QuantumRacer', 29, 87500, 'OFFLINE', 'https://cdn.example.com/avatars/quantum.png'),
    ('55555555-5555-5555-5555-555555555555', 'ThunderWolf', 61, 280000, 'ONLINE', 'https://cdn.example.com/avatars/thunder.png'),
    ('66666666-6666-6666-6666-666666666666', 'VoidWalker', 15, 32000, 'OFFLINE', 'https://cdn.example.com/avatars/void.png'),
    ('77777777-7777-7777-7777-777777777777', 'FrostByte', 33, 105000, 'IN_MATCH', 'https://cdn.example.com/avatars/frost.png'),
    ('88888888-8888-8888-8888-888888888888', 'IronTitan', 47, 168000, 'ONLINE', 'https://cdn.example.com/avatars/iron.png'),
    ('99999999-9999-9999-9999-999999999999', 'BlazeFury', 22, 58000, 'OFFLINE', 'https://cdn.example.com/avatars/blaze.png'),
    ('aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'SpectreGhost', 8, 12000, 'OFFLINE', 'https://cdn.example.com/avatars/spectre.png')
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- PLAYER SETTINGS
-- ============================================================================

INSERT INTO player_settings (player_id, graphics_quality, audio_enabled, music_volume, sfx_volume, push_notifications, language, timezone) VALUES
    ('11111111-1111-1111-1111-111111111111', 'ULTRA', TRUE, 70, 80, TRUE, 'en-US', 'America/New_York'),
    ('22222222-2222-2222-2222-222222222222', 'HIGH', TRUE, 60, 75, TRUE, 'en-US', 'Europe/London'),
    ('33333333-3333-3333-3333-333333333333', 'ULTRA', TRUE, 80, 90, TRUE, 'ja-JP', 'Asia/Tokyo'),
    ('44444444-4444-4444-4444-444444444444', 'MEDIUM', TRUE, 50, 60, FALSE, 'en-US', 'America/Los_Angeles'),
    ('55555555-5555-5555-5555-555555555555', 'LOW', FALSE, 0, 50, TRUE, 'ko-KR', 'Asia/Seoul'),
    ('66666666-6666-6666-6666-666666666666', 'MEDIUM', TRUE, 40, 70, TRUE, 'en-US', 'America/Chicago'),
    ('77777777-7777-7777-7777-777777777777', 'HIGH', TRUE, 65, 85, TRUE, 'de-DE', 'Europe/Berlin'),
    ('88888888-8888-8888-8888-888888888888', 'ULTRA', TRUE, 75, 95, TRUE, 'en-US', 'America/Denver'),
    ('99999999-9999-9999-9999-999999999999', 'MEDIUM', TRUE, 55, 65, FALSE, 'pt-BR', 'America/Sao_Paulo'),
    ('aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'LOW', TRUE, 30, 40, FALSE, 'en-US', 'America/New_York')
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- PLAYER STATISTICS
-- ============================================================================

INSERT INTO player_statistics (player_id, total_matches, total_wins, total_losses, total_kills, total_deaths, total_assists, total_score, total_playtime_seconds, rating) VALUES
    ('11111111-1111-1111-1111-111111111111', 1247, 687, 560, 15234, 9876, 11234, 45678900, 1894200, 1850),
    ('22222222-2222-2222-2222-222222222222', 1089, 534, 555, 12456, 10234, 9876, 38901200, 1654200, 1720),
    ('33333333-3333-3333-3333-333333333333', 2156, 1203, 953, 28901, 17654, 22345, 78901200, 3124500, 2100),
    ('44444444-4444-4444-4444-444444444444', 567, 278, 289, 6789, 6543, 5432, 15678000, 892300, 1450),
    ('55555555-5555-5555-5555-555555555555', 3421, 1987, 1434, 45678, 32345, 34567, 123456700, 4892300, 2250),
    ('66666666-6666-6666-6666-666666666666', 234, 89, 145, 2345, 2789, 1876, 4567800, 345600, 1150),
    ('77777777-7777-7777-7777-777777777777', 891, 423, 468, 10987, 9876, 8765, 27890100, 1345600, 1580),
    ('88888888-8888-8888-8888-888888888888', 1567, 845, 722, 19234, 14567, 15678, 56789000, 2345600, 1920),
    ('99999999-9999-9999-9999-999999999999', 456, 201, 255, 5678, 5234, 4567, 12345000, 678900, 1320),
    ('aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 89, 34, 55, 876, 1023, 654, 1234000, 134200, 1050)
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- PLAYER UNLOCKS
-- ============================================================================

INSERT INTO player_unlocks (id, player_id, unlock_type, unlock_id, source) VALUES
    ('u0000000-0000-0000-0000-000000000001', '11111111-1111-1111-1111-111111111111', 'SKIN', 'skin_legendary_001', 'PURCHASE'),
    ('u0000000-0000-0000-0000-000000000002', '11111111-1111-1111-1111-111111111111', 'EMOTE', 'emote_dance_001', 'ACHIEVEMENT'),
    ('u0000000-0000-0000-0000-000000000003', '33333333-3333-3333-3333-333333333333', 'SKIN', 'skin_epic_001', 'PURCHASE'),
    ('u0000000-0000-0000-0000-000000000004', '33333333-3333-3333-3333-333333333333', 'WEAPON', 'weapon_plasma_rifle', 'CRAFT'),
    ('u0000000-0000-0000-0000-000000000005', '55555555-5555-5555-5555-555555555555', 'SKIN', 'skin_legendary_002', 'SEASON_REWARD'),
    ('u0000000-0000-0000-0000-000000000006', '55555555-5555-5555-5555-555555555555', 'TITLE', 'title_legendary', 'ACHIEVEMENT'),
    ('u0000000-0000-0000-0000-000000000007', '88888888-8888-8888-8888-888888888888', 'MAP', 'map_desert_storm', 'PURCHASE'),
    ('u0000000-0000-0000-0000-000000000008', '88888888-8888-8888-8888-888888888888', 'SKIN', 'skin_rare_001', 'PURCHASE')
ON CONFLICT (player_id, unlock_type, unlock_id) DO NOTHING;

-- ============================================================================
-- BAN RECORDS
-- ============================================================================

INSERT INTO ban_records (id, account_id, banned_by, reason, started_at, expires_at) VALUES
    ('ban00000-0000-0000-0000-000000000001', 'b0000000-0000-0000-0000-000000000006', 'a0000000-0000-0000-0000-000000000002', 'Exploiting game mechanics', NOW() - INTERVAL '7 days', NULL),
    ('ban00000-0000-0000-0000-000000000002', 'b0000000-0000-0000-0000-000000000007', 'a0000000-0000-0000-0000-000000000003', 'Toxic behavior - verbal abuse', NOW() - INTERVAL '1 hour', NOW() + INTERVAL '29 minutes')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME ROOMS
-- ============================================================================

INSERT INTO game_rooms (id, name, mode, map_id, region, status, max_players, current_players, has_password, owner_id) VALUES
    ('r0000000-0000-0000-0000-000000000001', 'Neon''s Arena', 'RANKED', 'map_arena_001', 'NA_EAST', 'WAITING', 10, 4, FALSE, '11111111-1111-1111-1111-111111111111'),
    ('r0000000-0000-0000-0000-000000000002', 'Shadow Squad', 'CASUAL', 'map_desert_storm', 'EU_WEST', 'IN_PROGRESS', 8, 6, TRUE, '22222222-2222-2222-2222-222222222222'),
    ('r0000000-0000-0000-0000-000000000003', 'Phoenix Rising', 'RANKED', 'map_urban_night', 'NA_WEST', 'WAITING', 10, 2, FALSE, '33333333-3333-3333-3333-333333333333')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME ROOM PLAYERS
-- ============================================================================

INSERT INTO game_room_players (id, room_id, player_id, slot_index, team, is_ready, joined_at) VALUES
    ('rp000000-0000-0000-0000-0000000001', 'r0000000-0000-0000-0000-000000000001', '11111111-1111-1111-1111-111111111111', 0, 'ALPHA', TRUE, NOW() - INTERVAL '5 minutes'),
    ('rp000000-0000-0000-0000-0000000002', 'r0000000-0000-0000-0000-000000000001', '33333333-3333-3333-3333-333333333333', 1, 'ALPHA', TRUE, NOW() - INTERVAL '4 minutes'),
    ('rp000000-0000-0000-0000-0000000003', 'r0000000-0000-0000-0000-000000000001', '77777777-7777-7777-7777-777777777777', 2, 'BRAVO', TRUE, NOW() - INTERVAL '3 minutes'),
    ('rp000000-0000-0000-0000-0000000004', 'r0000000-0000-0000-0000-000000000001', '88888888-8888-8888-8888-888888888888', 3, 'BRAVO', FALSE, NOW() - INTERVAL '2 minutes')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- MATCHMAKING TICKETS
-- ============================================================================

INSERT INTO matchmaking_tickets (id, player_id, mode, region, mmr, status, timeout_at) VALUES
    ('mm000000-0000-0000-0000-000000000001', '99999999-9999-9999-9999-999999999999', 'RANKED', 'NA_EAST', 1320, 'QUEUED', NOW() + INTERVAL '5 minutes'),
    ('mm000000-0000-0000-0000-000000000002', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'CASUAL', 'NA_EAST', 1050, 'QUEUED', NOW() + INTERVAL '5 minutes')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME SERVER INSTANCES
-- ============================================================================

INSERT INTO game_server_instances (id, mode, map_id, region, build_version, ip, port, status, started_at) VALUES
    ('gs000000-0000-0000-0000-000000000001', 'RANKED', 'map_arena_001', 'NA_EAST', 'v1.2.3', '192.168.1.100', 7777, 'RUNNING', NOW() - INTERVAL '2 hours'),
    ('gs000000-0000-0000-0000-000000000002', 'CASUAL', 'map_desert_storm', 'EU_WEST', 'v1.2.3', '192.168.1.101', 7777, 'RUNNING', NOW() - INTERVAL '1 hour')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PORT ALLOCATIONS
-- ============================================================================

INSERT INTO port_allocations (port, status, server_id, allocated_at) VALUES
    (7777, 'ALLOCATED', 'gs000000-0000-0000-0000-000000000001', NOW() - INTERVAL '2 hours'),
    (7778, 'ALLOCATED', 'gs000000-0000-0000-0000-000000000002', NOW() - INTERVAL '1 hour'),
    (7779, 'FREE', NULL, NULL)
ON CONFLICT (port) DO NOTHING;

-- ============================================================================
-- GAME SESSIONS
-- ============================================================================

INSERT INTO game_sessions (id, source_type, source_id, mode, map_id, region, status, server_id, max_players, started_at) VALUES
    ('s0000000-0000-0000-0000-000000000001', 'ROOM', 'r0000000-0000-0000-0000-000000000002', 'CASUAL', 'map_desert_storm', 'EU_WEST', 'IN_PROGRESS', 'gs000000-0000-0000-0000-000000000002', 8, NOW() - INTERVAL '45 minutes'),
    ('s0000000-0000-0000-0000-000000000002', 'MATCHMAKING', 'mm000000-0000-0000-0000-000000000001', 'RANKED', 'map_arena_001', 'NA_EAST', 'CREATED', NULL, 10, NULL)
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PLAYER SESSIONS
-- ============================================================================

INSERT INTO player_sessions (id, game_session_id, player_id, slot_index, team, status, joined_at) VALUES
    ('ps000000-0000-0000-0000-0000000001', 's0000000-0000-0000-0000-000000000001', '22222222-2222-2222-2222-222222222222', 0, 'ALPHA', 'CONNECTED', NOW() - INTERVAL '44 minutes'),
    ('ps000000-0000-0000-0000-0000000002', 's0000000-0000-0000-0000-000000000001', '77777777-7777-7777-7777-777777777777', 1, 'BRAVO', 'CONNECTED', NOW() - INTERVAL '44 minutes')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- INVENTORY ITEMS
-- ============================================================================

INSERT INTO inventory_items (id, player_id, item_id, quantity, expires_at) VALUES
    ('inv00000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'gold_coin_pack_100', 5, NULL),
    ('inv00000-0000-0000-0000-0000000002', '11111111-1111-1111-1111-111111111111', 'xp_boost_2x', 3, NOW() + INTERVAL '7 days'),
    ('inv00000-0000-0000-0000-0000000003', '33333333-3333-3333-3333-333333333333', 'gold_coin_pack_500', 2, NULL),
    ('inv00000-0000-0000-0000-0000000004', '55555555-5555-5555-5555-555555555555', 'season_pass_premium', 1, NOW() + INTERVAL '45 days'),
    ('inv00000-0000-0000-0000-0000000005', '88888888-8888-8888-8888-888888888888', 'xp_boost_2x', 10, NULL)
ON CONFLICT (player_id, item_id) DO NOTHING;

-- ============================================================================
-- WALLET BALANCES
-- ============================================================================

INSERT INTO wallet_balances (id, player_id, currency_type, balance) VALUES
    ('wb000000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'GOLD', 15000),
    ('wb000000-0000-0000-0000-0000000002', '33333333-3333-3333-3333-333333333333', 'GOLD', 25000),
    ('wb000000-0000-0000-0000-0000000003', '55555555-5555-5555-5555-555555555555', 'GOLD', 50000),
    ('wb000000-0000-0000-0000-0000000004', '88888888-8888-8888-8888-888888888888', 'GOLD', 8500)
ON CONFLICT (player_id, currency_type) DO NOTHING;

-- ============================================================================
-- CRASH REPORTS
-- ============================================================================

INSERT INTO crash_reports (id, player_id, client_version, platform, crash_type, title, description, metadata_json) VALUES
    ('cr000000-0000-0000-0000-0000000001', '44444444-4444-4444-4444-444444444444', 'v1.2.3', 'PC', 'RENDER_FATAL', 'GPU device lost', 'Graphics device completely lost during intensive scene rendering', '{"gpu_model": "NVIDIA RTX 3080", "driver_version": "536.23"}'),
    ('cr000000-0000-0000-0000-0000000002', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'v1.2.2', 'CONSOLE', 'OUT_OF_MEMORY', 'Memory allocation failed', 'Failed to allocate 16GB texture pool', '{"memory_pool_size": "8GB", "platform": "XSX"}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PLAYER FEEDBACK
-- ============================================================================

INSERT INTO player_feedback (id, player_id, nickname, email, feedback_type, title, content, status) VALUES
    ('fb000000-0000-0000-0000-0000000001', '44444444-4444-4444-4444-444444444444', 'QuantumRacer', 'player4@example.com', 'BUG_REPORT', 'Match disconnects on desert map', 'I keep getting disconnected specifically when loading into the desert_storm map. This happens 9 out of 10 times.', 'OPEN'),
    ('fb000000-0000-0000-0000-0000000002', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'SpectreGhost', 'player10@example.com', 'FEATURE_REQUEST', 'Add more character customization options', 'Would love to see more hairstyles and face options for character creation.', 'IN_REVIEW')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- ORDER RECORDS
-- ============================================================================

INSERT INTO order_records (id, player_id, platform, platform_order_id, status, amount, currency, item_json, paid_at) VALUES
    ('or000000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'STEAM', 'STEAM-ORD-001', 'COMPLETED', 999, 'USD', '{"item_ids": ["gold_coin_pack_100"], "quantity": 5}', NOW() - INTERVAL '2 days'),
    ('or000000-0000-0000-0000-0000000002', '33333333-3333-3333-3333-333333333333', 'EPIC', 'EPIC-ORD-002', 'COMPLETED', 4999, 'USD', '{"item_ids": ["gold_coin_pack_500"], "quantity": 2}', NOW() - INTERVAL '5 days'),
    ('or000000-0000-0000-0000-0000000003', '55555555-5555-5555-5555-555555555555', 'STEAM', 'STEAM-ORD-003', 'PENDING', 1499, 'USD', '{"item_ids": ["season_pass_premium"], "quantity": 1}', NULL)
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- PLAYER EVENT LOGS (sample for player 1)
-- ============================================================================

INSERT INTO player_event_logs (id, player_id, event_type, payload_json) VALUES
    ('ev000000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'LEVEL_UP', '{"new_level": 42, "exp_gained": 500}'),
    ('ev000000-0000-0000-0000-0000000002', '11111111-1111-1111-1111-111111111111', 'MATCH_COMPLETE', '{"mode": "RANKED", "result": "WIN", "kills": 12, "deaths": 5}'),
    ('ev000000-0000-0000-0000-0000000003', '33333333-3333-3333-3333-333333333333', 'ACHIEVEMENT_UNLOCKED', '{"achievement_id": "first_blood", "achievement_name": "First Blood"}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME CONFIG PUBLISH LOGS
-- ============================================================================

INSERT INTO game_config_publish_logs (id, config_id, action, before_json, after_json, admin_user_id, reason) VALUES
    ('gcpl0000-0000-0000-0000-000000001', 'c0000000-0000-0000-0000-000000000001', 'PUBLISH', NULL, '{"queue_timeout_seconds": 300}', 'a0000000-0000-0000-0000-000000000001', 'Initial publish for season 1'),
    ('gcpl0000-0000-0000-0000-000000002', 'c0000000-0000-0000-0000-000000000002', 'PUBLISH', NULL, '{"season_id": "season_1"}', 'a0000000-0000-0000-0000-000000000001', 'Activate ranked season 1')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- ADMIN AUDIT LOGS
-- ============================================================================

INSERT INTO admin_audit_logs (id, admin_user_id, action, target_type, target_id, reason, before_json, after_json) VALUES
    ('aal00000-0000-0000-0000-0000000001', 'a0000000-0000-0000-0000-000000000002', 'BAN_PLAYER', 'ACCOUNT', 'b0000000-0000-0000-0000-000000000006', 'Exploiting game mechanics', NULL, '{"status": "BANNED", "expires_at": null}'),
    ('aal00000-0000-0000-0000-0000000002', 'a0000000-0000-0000-0000-000000000001', 'PUBLISH_CONFIG', 'GAME_CONFIG', 'c0000000-0000-0000-0000-000000000001', 'Initial publish', NULL, '{"status": "PUBLISHED"}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- SESSION EVENTS (sample)
-- ============================================================================

INSERT INTO session_events (id, game_session_id, event_type, payload_json) VALUES
    ('se000000-0000-0000-0000-0000000001', 's0000000-0000-0000-0000-000000000001', 'MATCH_START', '{"timestamp": "2026-05-16T10:00:00Z"}'),
    ('se000000-0000-0000-0000-0000000002', 's0000000-0000-0000-0000-000000000001', 'MATCH_END', '{"timestamp": "2026-05-16T10:45:00Z", "duration_seconds": 2700}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- GAME SERVER EVENTS (sample)
-- ============================================================================

INSERT INTO game_server_events (id, server_id, event_type, payload_json) VALUES
    ('gse00000-0000-0000-0000-0000000001', 'gs000000-0000-0000-0000-000000000001', 'SERVER_READY', '{"players_connected": 0}'),
    ('gse00000-0000-0000-0000-0000000002', 'gs000000-0000-0000-0000-000000000002', 'PLAYER_JOINED', '{"player_id": "22222222-2222-2222-2222-222222222222", "slot_index": 0}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- MATCH RESULTS
-- ============================================================================

INSERT INTO match_results (id, session_id, server_id, mode, map_id, duration_seconds, result_json, idempotency_key) VALUES
    ('mr000000-0000-0000-0000-0000000001', 's0000000-0000-0000-0000-000000000001', 'gs000000-0000-0000-0000-000000000002', 'CASUAL', 'map_desert_storm', 2700, '{"winner": "ALPHA", "score": {"ALPHA": 150, "BRAVO": 120}}', 'match_result_casual_001')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- MATCH PLAYER RESULTS
-- ============================================================================

INSERT INTO match_player_results (id, match_result_id, player_id, team, result, kills, deaths, assists, score, exp_delta, reward_json) VALUES
    ('mpr00000-0000-0000-0000-000000001', 'mr000000-0000-0000-0000-0000000001', '22222222-2222-2222-2222-222222222222', 'ALPHA', 'WIN', 15, 7, 12, 2450, 500, '{"exp": 500, "gold": 150}'),
    ('mpr00000-0000-0000-0000-000000002', 'mr000000-0000-0000-0000-0000000001', '77777777-7777-7777-7777-777777777777', 'BRAVO', 'LOSS', 8, 12, 9, 1850, 200, '{"exp": 200, "gold": 100}')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- REFRESH TOKENS (sample)
-- ============================================================================

INSERT INTO refresh_tokens (id, account_id, token_hash, device_info, ip_address, expires_at) VALUES
    ('rt000000-0000-0000-0000-0000000001', 'b0000000-0000-0000-0000-000000000001', '$2b$12$hashed_token_value_here_for_player1', 'Chrome/Windows', '192.168.1.50', NOW() + INTERVAL '30 days'),
    ('rt000000-0000-0000-0000-0000000002', 'b0000000-0000-0000-0000-000000000002', '$2b$12$hashed_token_value_here_for_player2', 'Firefox/MacOS', '192.168.1.51', NOW() + INTERVAL '30 days')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- DEVICE LOGINS (sample)
-- ============================================================================

INSERT INTO device_logins (id, account_id, device_id, device_name, last_login_at) VALUES
    ('dl000000-0000-0000-0000-0000000001', 'b0000000-0000-0000-0000-000000000001', 'device_pc_001', 'Gaming PC', NOW() - INTERVAL '1 hour'),
    ('dl000000-0000-0000-0000-0000000002', 'b0000000-0000-0000-0000-000000000001', 'device_mobile_001', 'iPhone 15 Pro', NOW() - INTERVAL '2 hours')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- INVENTORY LOGS (sample)
-- ============================================================================

INSERT INTO inventory_logs (id, player_id, item_id, quantity_delta, quantity_before, quantity_after, reason, biz_type, biz_id) VALUES
    ('il000000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'gold_coin_pack_100', 5, 0, 5, 'PURCHASE', 'ORDER', 'or000000-0000-0000-0000-0000000001'),
    ('il000000-0000-0000-0000-0000000002', '11111111-1111-1111-1111-111111111111', 'gold_coin_pack_100', -2, 5, 3, 'USE', 'GAMEplay', 'match_001')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- WALLET LEDGER (sample)
-- ============================================================================

INSERT INTO wallet_ledger (id, player_id, currency_type, amount, balance_before, balance_after, biz_type, biz_id, idempotency_key) VALUES
    ('wl000000-0000-0000-0000-0000000001', '11111111-1111-1111-1111-111111111111', 'GOLD', 5000, 10000, 15000, 'PURCHASE', 'ORDER', 'or000000-0000-0000-0000-0000000001', 'wallet_tx_001'),
    ('wl000000-0000-0000-0000-0000000002', '11111111-1111-1111-1111-111111111111', 'GOLD', -1500, 15000, 13500, 'PURCHASE', 'INVENTORY', 'inv_trade_001', 'wallet_tx_002')
ON CONFLICT (id) DO NOTHING;