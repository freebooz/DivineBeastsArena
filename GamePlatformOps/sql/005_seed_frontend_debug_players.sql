-- 005_seed_frontend_debug_players.sql
-- Additional player seeds for frontend debugging and QA testing
-- Execution: AFTER 001_create_schema.sql and 002_seed_mock_data.sql (optional)
-- Purpose: Provides stable, well-known player accounts for UI development and testing

-- ============================================================================
-- DEBUG ACCOUNTS (emails prefixed with debug_ for easy identification)
-- ============================================================================

INSERT INTO accounts (id, email, password_hash, status, failed_login_count) VALUES
    ('debug000-0000-0000-0000-000000000001', 'debug_player_alpha@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000002', 'debug_player_beta@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000003', 'debug_player_gamma@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000004', 'debug_player_delta@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000005', 'debug_player_epsilon@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000006', 'debug_player_zeta@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000007', 'debug_player_eta@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0),
    ('debug000-0000-0000-0000-000000000008', 'debug_player_theta@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4j0i7J.qXC9ETMmK', 'ACTIVE', 0)
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- DEBUG PLAYER IDENTITIES
-- Debug player IDs use pattern: debugXXX-... for easy identification in logs
-- ============================================================================

INSERT INTO player_identities (id, account_id, player_id, display_name) VALUES
    ('dp000000-0000-0000-0000-000000000001', 'debug000-0000-0000-0000-000000000001', 'debug001-debug-0001-0000-000000001', 'DebugAlpha'),
    ('dp000000-0000-0000-0000-000000000002', 'debug000-0000-0000-0000-000000000002', 'debug002-debug-0002-0000-000000002', 'DebugBeta'),
    ('dp000000-0000-0000-0000-000000000003', 'debug000-0000-0000-0000-000000000003', 'debug003-debug-0003-0000-000000003', 'DebugGamma'),
    ('dp000000-0000-0000-0000-000000000004', 'debug000-0000-0000-0000-000000000004', 'debug004-debug-0004-0000-000000004', 'DebugDelta'),
    ('dp000000-0000-0000-0000-000000000005', 'debug000-0000-0000-0000-000000000005', 'debug005-debug-0005-0000-000000005', 'DebugEpsilon'),
    ('dp000000-0000-0000-0000-000000000006', 'debug000-0000-0000-0000-000000000006', 'debug006-debug-0006-0000-000000006', 'DebugZeta'),
    ('dp000000-0000-0000-0000-000000000007', 'debug000-0000-0000-0000-000000000007', 'debug007-debug-0007-0000-000000007', 'DebugEta'),
    ('dp000000-0000-0000-0000-000000000008', 'debug000-0000-0000-0000-000000000008', 'debug008-debug-0008-0000-000000008', 'DebugTheta')
ON CONFLICT (id) DO NOTHING;

-- ============================================================================
-- DEBUG PLAYER PROFILES
-- ============================================================================

INSERT INTO player_profiles (player_id, nickname, level, exp, status, avatar_url) VALUES
    ('debug001-debug-0001-0000-000000001', 'DebugAlpha', 1, 0, 'OFFLINE', NULL),
    ('debug002-debug-0002-0000-000000002', 'DebugBeta', 10, 25000, 'ONLINE', NULL),
    ('debug003-debug-0003-0000-000000003', 'DebugGamma', 25, 75000, 'IN_MATCH', NULL),
    ('debug004-debug-0004-0000-000000004', 'DebugDelta', 50, 175000, 'OFFLINE', NULL),
    ('debug005-debug-0005-0000-000000005', 'DebugEpsilon', 75, 275000, 'ONLINE', NULL),
    ('debug006-debug-0006-0000-000000006', 'DebugZeta', 100, 400000, 'ONLINE', NULL),
    ('debug007-debug-0007-0000-000000007', 'DebugEta', 1, 100, 'OFFLINE', NULL),
    ('debug008-debug-0008-0000-000000008', 'DebugTheta', 99, 385000, 'IN_MATCH', NULL)
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- DEBUG PLAYER SETTINGS (defaults for fresh debug accounts)
-- ============================================================================

INSERT INTO player_settings (player_id, graphics_quality, audio_enabled, music_volume, sfx_volume, push_notifications, language, timezone) VALUES
    ('debug001-debug-0001-0000-000000001', 'LOW', TRUE, 50, 50, FALSE, 'en-US', 'UTC'),
    ('debug002-debug-0002-0000-000000002', 'MEDIUM', TRUE, 50, 50, TRUE, 'en-US', 'UTC'),
    ('debug003-debug-0003-0000-000000003', 'HIGH', TRUE, 60, 70, TRUE, 'en-US', 'UTC'),
    ('debug004-debug-0004-0000-000000004', 'ULTRA', TRUE, 80, 90, TRUE, 'en-US', 'UTC'),
    ('debug005-debug-0005-0000-000000005', 'LOW', FALSE, 0, 50, FALSE, 'en-US', 'UTC'),
    ('debug006-debug-0006-0000-000000006', 'MEDIUM', TRUE, 50, 60, TRUE, 'en-US', 'UTC'),
    ('debug007-debug-0007-0000-000000007', 'LOW', TRUE, 30, 40, FALSE, 'en-US', 'UTC'),
    ('debug008-debug-0008-0000-000000008', 'HIGH', TRUE, 70, 80, TRUE, 'en-US', 'UTC')
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- DEBUG PLAYER STATISTICS
-- ============================================================================

INSERT INTO player_statistics (player_id, total_matches, total_wins, total_losses, total_kills, total_deaths, total_assists, total_score, total_playtime_seconds, rating) VALUES
    ('debug001-debug-0001-0000-000000001', 0, 0, 0, 0, 0, 0, 0, 0, 1000),
    ('debug002-debug-0002-0000-000000002', 50, 25, 25, 500, 450, 300, 750000, 75000, 1100),
    ('debug003-debug-0003-0000-000000003', 200, 100, 100, 2500, 2200, 1500, 3000000, 300000, 1300),
    ('debug004-debug-0004-0000-000000004', 500, 250, 250, 7500, 6500, 5000, 7500000, 750000, 1600),
    ('debug005-debug-0005-0000-000000005', 1000, 550, 450, 15000, 12000, 10000, 15000000, 1500000, 1900),
    ('debug006-debug-0006-0000-000000006', 2000, 1100, 900, 35000, 28000, 22000, 35000000, 3000000, 2200),
    ('debug007-debug-0007-0000-000000007', 1, 0, 1, 5, 10, 3, 5000, 1800, 950),
    ('debug008-debug-0008-0000-000000008', 1500, 750, 750, 20000, 18000, 15000, 22500000, 2250000, 2050)
ON CONFLICT (player_id) DO NOTHING;

-- ============================================================================
-- DEBUG PLAYER UNLOCKS (varied unlock states for UI testing)
-- ============================================================================

INSERT INTO player_unlocks (id, player_id, unlock_type, unlock_id, source) VALUES
    ('dpun0000-0000-0000-0000-0000000001', 'debug001-debug-0001-0000-000000001', 'SKIN', 'skin_starter_pack', 'DEFAULT'),
    ('dpun0000-0000-0000-0000-0000000002', 'debug004-debug-0004-0000-000000004', 'SKIN', 'skin_common_001', 'PURCHASE'),
    ('dpun0000-0000-0000-0000-0000000003', 'debug004-debug-0004-0000-000000004', 'EMOTE', 'emote_wave_001', 'ACHIEVEMENT'),
    ('dpun0000-0000-0000-0000-0000000004', 'debug006-debug-0006-0000-000000006', 'SKIN', 'skin_legendary_001', 'SEASON_REWARD'),
    ('dpun0000-0000-0000-0000-0000000005', 'debug006-debug-0006-0000-000000006', 'SKIN', 'skin_epic_001', 'PURCHASE'),
    ('dpun0000-0000-0000-0000-0000000006', 'debug006-debug-0006-0000-000000006', 'TITLE', 'title_veteran', 'ACHIEVEMENT'),
    ('dpun0000-0000-0000-0000-0000000007', 'debug008-debug-0008-0000-000000008', 'SKIN', 'skin_rare_001', 'PURCHASE')
ON CONFLICT (player_id, unlock_type, unlock_id) DO NOTHING;

-- ============================================================================
-- DEBUG WALLET BALANCES
-- ============================================================================

INSERT INTO wallet_balances (id, player_id, currency_type, balance) VALUES
    ('dwb00000-0000-0000-0000-0000000001', 'debug001-debug-0001-0000-000000001', 'GOLD', 0),
    ('dwb00000-0000-0000-0000-0000000002', 'debug002-debug-0002-0000-000000002', 'GOLD', 500),
    ('dwb00000-0000-0000-0000-0000000003', 'debug003-debug-0003-0000-000000003', 'GOLD', 2500),
    ('dwb00000-0000-0000-0000-0000000004', 'debug004-debug-0004-0000-000000004', 'GOLD', 10000),
    ('dwb00000-0000-0000-0000-0000000005', 'debug005-debug-0005-0000-000000005', 'GOLD', 25000),
    ('dwb00000-0000-0000-0000-0000000006', 'debug006-debug-0006-0000-000000006', 'GOLD', 100000),
    ('dwb00000-0000-0000-0000-0000000007', 'debug007-debug-0007-0000-000000007', 'GOLD', 100),
    ('dwb00000-0000-0000-0000-0000000008', 'debug008-debug-0008-0000-000000008', 'GOLD', 50000)
ON CONFLICT (player_id, currency_type) DO NOTHING;

-- ============================================================================
-- DEBUG INVENTORY ITEMS (varied for testing inventory UI states)
-- ============================================================================

INSERT INTO inventory_items (id, player_id, item_id, quantity, expires_at) VALUES
    ('dinv0000-0000-0000-0000-0000000001', 'debug001-debug-0001-0000-000000001', 'xp_boost_2x', 1, NULL),
    ('dinv0000-0000-0000-0000-0000000002', 'debug004-debug-0004-0000-000000004', 'gold_coin_pack_100', 3, NULL),
    ('dinv0000-0000-0000-0000-0000000003', 'debug004-debug-0004-0000-000000004', 'xp_boost_2x', 5, NOW() + INTERVAL '3 days'),
    ('dinv0000-0000-0000-0000-0000000004', 'debug006-debug-0006-0000-000000006', 'gold_coin_pack_500', 10, NULL),
    ('dinv0000-0000-0000-0000-0000000005', 'debug006-debug-0006-0000-000000006', 'xp_boost_2x', 20, NOW() + INTERVAL '14 days'),
    ('dinv0000-0000-0000-0000-0000000006', 'debug006-debug-0006-0000-000000006', 'season_pass_premium', 1, NOW() + INTERVAL '60 days'),
    ('dinv0000-0000-0000-0000-0000000007', 'debug008-debug-0008-0000-000000008', 'gold_coin_pack_100', 7, NOW() + INTERVAL '1 day')
ON CONFLICT (player_id, item_id) DO NOTHING;