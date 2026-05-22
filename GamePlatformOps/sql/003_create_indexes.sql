-- 003_create_indexes.sql
-- Additional indexes beyond 001_create_schema.sql
-- Execution: AFTER 001_create_schema.sql and optionally 002_seed_mock_data.sql

-- ============================================================================
-- COMPOSITE INDEXES FOR COMMON QUERY PATTERNS
-- ============================================================================

-- Player lookup by identity (account + player)
CREATE INDEX idx_player_identities_account_player ON player_identities(account_id, player_id);

-- Active ban check (account + not expired)
CREATE INDEX idx_ban_records_account_active ON ban_records(account_id, expires_at) WHERE expires_at IS NULL OR expires_at > NOW();

-- Player profile status + level for leaderboards
CREATE INDEX idx_player_profiles_status_level ON player_profiles(status, level DESC);

-- Player statistics rating for MMR-based matchmaking
CREATE INDEX idx_player_statistics_rating ON player_statistics(rating DESC);

-- Player unlocks composite (player + type)
CREATE INDEX idx_player_unlocks_player_type ON player_unlocks(player_id, unlock_type);

-- Player event logs by type and time
CREATE INDEX idx_player_event_logs_type_created ON player_event_logs(event_type, created_at DESC);

-- ============================================================================
-- GAME CONFIGS QUERY OPTIMIZATION
-- ============================================================================

-- Latest version of a config key (for hot path lookups)
CREATE INDEX idx_game_configs_key_version ON game_configs(config_key, version DESC);
CREATE INDEX idx_game_configs_key_status ON game_configs(config_key, status) WHERE status = 'PUBLISHED';

-- Config publish logs by config and time
CREATE INDEX idx_game_config_publish_logs_config_time ON game_config_publish_logs(config_id, created_at DESC);

-- ============================================================================
-- GAME ROOMS QUERY OPTIMIZATION
-- ============================================================================

-- Available rooms by mode/region (excluding full/in-progress)
CREATE INDEX idx_game_rooms_mode_region_available ON game_rooms(mode, region, status) WHERE status = 'WAITING' AND current_players < max_players;

-- Rooms by owner for quick ownership lookup
CREATE INDEX idx_game_rooms_owner ON game_rooms(owner_id);

-- Game room players by slot
CREATE INDEX idx_game_room_players_room_slot ON game_room_players(room_id, slot_index);

-- ============================================================================
-- MATCHMAKING QUERY OPTIMIZATION
-- ============================================================================

-- Active tickets by mode/region/MMR for matchmaker
CREATE INDEX idx_matchmaking_tickets_queue ON matchmaking_tickets(mode, region, mmr) WHERE status = 'QUEUED';

-- Tickets by player for duplicate check
CREATE INDEX idx_matchmaking_tickets_player_status ON matchmaking_tickets(player_id, status) WHERE status = 'QUEUED';

-- ============================================================================
-- GAME SESSIONS QUERY OPTIMIZATION
-- ============================================================================

-- Sessions by status and allocation time (for stale session cleanup)
CREATE INDEX idx_game_sessions_status_allocated ON game_sessions(status, allocated_at) WHERE status = 'CREATED';

-- Player sessions by player and status
CREATE INDEX idx_player_sessions_player_status ON player_sessions(player_id, status);

-- Session events by type for analytics
CREATE INDEX idx_session_events_type_time ON session_events(event_type, created_at DESC);

-- ============================================================================
-- GAME SERVER INSTANCES QUERY OPTIMIZATION
-- ============================================================================

-- Servers by status and last heartbeat (for health monitoring)
CREATE INDEX idx_game_server_instances_status_heartbeat ON game_server_instances(status, last_heartbeat_at) WHERE status = 'RUNNING';

-- Server events by type and time
CREATE INDEX idx_game_server_events_type_time ON game_server_events(event_type, created_at DESC);

-- Port allocations by status for quick availability check
CREATE INDEX idx_port_allocations_status ON port_allocations(status) WHERE status = 'FREE';

-- ============================================================================
-- MATCH RESULTS QUERY OPTIMIZATION
-- ============================================================================

-- Match results by server and time
CREATE INDEX idx_match_results_server_created ON match_results(server_id, created_at DESC);

-- Match player results by player and time
CREATE INDEX idx_match_player_results_player_created ON match_player_results(player_id, created_at DESC);

-- ============================================================================
-- INVENTORY QUERY OPTIMIZATION
-- ============================================================================

-- Inventory items by item_id (for item metadata lookups)
CREATE INDEX idx_inventory_items_item_id ON inventory_items(item_id);

-- Non-expired items for cleanup jobs
CREATE INDEX idx_inventory_items_expires ON inventory_items(expires_at) WHERE expires_at IS NOT NULL AND expires_at > NOW();

-- Inventory logs by biz identifiers
CREATE INDEX idx_inventory_logs_biz ON inventory_logs(biz_type, biz_id);

-- ============================================================================
-- ADMIN AUDIT LOG QUERY OPTIMIZATION
-- ============================================================================

-- Audit logs by target for reverse lookup
CREATE INDEX idx_admin_audit_logs_target ON admin_audit_logs(target_type, target_id);

-- Audit logs by admin and time
CREATE INDEX idx_admin_audit_logs_admin_time ON admin_audit_logs(admin_user_id, created_at DESC);

-- Audit logs by action type for analytics
CREATE INDEX idx_admin_audit_logs_action_time ON admin_audit_logs(action, created_at DESC);

-- ============================================================================
-- SUPPORT TABLES QUERY OPTIMIZATION
-- ============================================================================

-- Crash reports by platform and time
CREATE INDEX idx_crash_reports_platform_created ON crash_reports(platform, created_at DESC);

-- Player feedback by status and created time
CREATE INDEX idx_player_feedback_status_created ON player_feedback(status, created_at DESC);

-- Order records by platform and status
CREATE INDEX idx_order_records_platform_status ON order_records(platform, status);

-- Order records by paid time for revenue reports
CREATE INDEX idx_order_records_paid ON order_records(paid_at DESC) WHERE paid_at IS NOT NULL;

-- ============================================================================
-- WALLET QUERY OPTIMIZATION
-- ============================================================================

-- Wallet ledger by currency type for balance reconciliation
CREATE INDEX idx_wallet_ledger_currency_time ON wallet_ledger(currency_type, created_at DESC);

-- Wallet ledger by biz identifiers
CREATE INDEX idx_wallet_ledger_biz ON wallet_ledger(biz_type, biz_id);