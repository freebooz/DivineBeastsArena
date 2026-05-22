-- MyGamePlatform Database Schema
-- Initial schema for all entities

-- Accounts and Auth
CREATE TABLE IF NOT EXISTS accounts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    email VARCHAR(255) UNIQUE,
    password_hash VARCHAR(255),
    status VARCHAR(50) NOT NULL DEFAULT 'ACTIVE',
    failed_login_count INTEGER NOT NULL DEFAULT 0,
    locked_until TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_identities (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
    player_id UUID NOT NULL UNIQUE,
    display_name VARCHAR(100) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS refresh_tokens (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
    token_hash VARCHAR(255) NOT NULL,
    device_info VARCHAR(500),
    ip_address VARCHAR(100),
    expires_at TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    revoked_at TIMESTAMPTZ
);

CREATE TABLE IF NOT EXISTS device_logins (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
    device_id VARCHAR(255) NOT NULL,
    device_name VARCHAR(255),
    last_login_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS ban_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
    banned_by UUID,
    reason TEXT,
    started_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    expires_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Player Profile
CREATE TABLE IF NOT EXISTS player_profiles (
    player_id UUID PRIMARY KEY,
    nickname VARCHAR(50) NOT NULL UNIQUE,
    level INTEGER NOT NULL DEFAULT 1,
    exp BIGINT NOT NULL DEFAULT 0,
    status VARCHAR(50) NOT NULL DEFAULT 'OFFLINE',
    avatar_url VARCHAR(500),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_settings (
    player_id UUID PRIMARY KEY REFERENCES player_profiles(player_id) ON DELETE CASCADE,
    graphics_quality VARCHAR(50) NOT NULL DEFAULT 'MEDIUM',
    audio_enabled BOOLEAN NOT NULL DEFAULT TRUE,
    music_volume INTEGER NOT NULL DEFAULT 50,
    sfx_volume INTEGER NOT NULL DEFAULT 50,
    push_notifications BOOLEAN NOT NULL DEFAULT TRUE,
    language VARCHAR(20) NOT NULL DEFAULT 'en-US',
    timezone VARCHAR(100),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_statistics (
    player_id UUID PRIMARY KEY REFERENCES player_profiles(player_id) ON DELETE CASCADE,
    total_matches INTEGER NOT NULL DEFAULT 0,
    total_wins INTEGER NOT NULL DEFAULT 0,
    total_losses INTEGER NOT NULL DEFAULT 0,
    total_kills INTEGER NOT NULL DEFAULT 0,
    total_deaths INTEGER NOT NULL DEFAULT 0,
    total_assists INTEGER NOT NULL DEFAULT 0,
    total_score BIGINT NOT NULL DEFAULT 0,
    total_playtime_seconds BIGINT NOT NULL DEFAULT 0,
    rating MMR NOT NULL DEFAULT 1000,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_unlocks (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL REFERENCES player_profiles(player_id) ON DELETE CASCADE,
    unlock_type VARCHAR(50) NOT NULL,
    unlock_id VARCHAR(255) NOT NULL,
    source VARCHAR(50) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(player_id, unlock_type, unlock_id)
);

CREATE TABLE IF NOT EXISTS player_event_logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL REFERENCES player_profiles(player_id) ON DELETE CASCADE,
    event_type VARCHAR(100) NOT NULL,
    payload_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Game Config
CREATE TABLE IF NOT EXISTS game_configs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    config_key VARCHAR(255) NOT NULL,
    config_type VARCHAR(50) NOT NULL,
    config_value JSONB NOT NULL,
    version INTEGER NOT NULL DEFAULT 1,
    status VARCHAR(50) NOT NULL DEFAULT 'DRAFT',
    checksum VARCHAR(255),
    created_by UUID,
    published_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(config_key, version)
);

CREATE TABLE IF NOT EXISTS game_config_publish_logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    config_id UUID NOT NULL REFERENCES game_configs(id),
    action VARCHAR(50) NOT NULL,
    before_json JSONB,
    after_json JSONB,
    admin_user_id UUID,
    reason TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS admin_audit_logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    admin_user_id UUID,
    action VARCHAR(100) NOT NULL,
    target_type VARCHAR(100),
    target_id VARCHAR(255),
    reason TEXT,
    before_json JSONB,
    after_json JSONB,
    ip_address VARCHAR(100),
    user_agent VARCHAR(500),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Room and Match
CREATE TABLE IF NOT EXISTS game_rooms (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(100) NOT NULL,
    mode VARCHAR(50) NOT NULL,
    map_id VARCHAR(100) NOT NULL,
    region VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'WAITING',
    max_players INTEGER NOT NULL DEFAULT 10,
    current_players INTEGER NOT NULL DEFAULT 0,
    has_password BOOLEAN NOT NULL DEFAULT FALSE,
    owner_id UUID NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS game_room_players (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    room_id UUID NOT NULL REFERENCES game_rooms(id) ON DELETE CASCADE,
    player_id UUID NOT NULL,
    slot_index INTEGER NOT NULL,
    team VARCHAR(50),
    is_ready BOOLEAN NOT NULL DEFAULT FALSE,
    joined_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    left_at TIMESTAMPTZ,
    UNIQUE(room_id, player_id)
);

CREATE TABLE IF NOT EXISTS matchmaking_tickets (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL,
    mode VARCHAR(50) NOT NULL,
    region VARCHAR(50) NOT NULL,
    mmr INTEGER NOT NULL DEFAULT 1000,
    status VARCHAR(50) NOT NULL DEFAULT 'QUEUED',
    matched_session_id UUID,
    timeout_at TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    cancelled_at TIMESTAMPTZ
);

-- Session
CREATE TABLE IF NOT EXISTS game_sessions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    source_type VARCHAR(50) NOT NULL,
    source_id UUID,
    mode VARCHAR(50) NOT NULL,
    map_id VARCHAR(100) NOT NULL,
    region VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'CREATED',
    server_id UUID,
    server_ip VARCHAR(100),
    server_port INTEGER,
    max_players INTEGER NOT NULL DEFAULT 10,
    retry_count INTEGER NOT NULL DEFAULT 0,
    allocated_at TIMESTAMPTZ,
    started_at TIMESTAMPTZ,
    ended_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_sessions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    game_session_id UUID NOT NULL REFERENCES game_sessions(id) ON DELETE CASCADE,
    player_id UUID NOT NULL,
    slot_index INTEGER NOT NULL,
    team VARCHAR(50),
    status VARCHAR(50) NOT NULL DEFAULT 'CREATED',
    session_token_hash VARCHAR(255),
    session_token_expires_at TIMESTAMPTZ,
    reconnect_token_hash VARCHAR(255),
    reconnect_token_expires_at TIMESTAMPTZ,
    joined_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    left_at TIMESTAMPTZ,
    UNIQUE(game_session_id, player_id)
);

CREATE TABLE IF NOT EXISTS session_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    game_session_id UUID NOT NULL REFERENCES game_sessions(id) ON DELETE CASCADE,
    event_type VARCHAR(100) NOT NULL,
    payload_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Game Server
CREATE TABLE IF NOT EXISTS game_server_instances (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    session_id UUID,
    mode VARCHAR(50),
    map_id VARCHAR(100),
    region VARCHAR(50),
    build_version VARCHAR(50),
    ip VARCHAR(100) NOT NULL,
    port INTEGER NOT NULL,
    process_id INTEGER,
    container_id VARCHAR(255),
    runtime_token_hash VARCHAR(255),
    runtime_token_expires_at TIMESTAMPTZ,
    status VARCHAR(50) NOT NULL DEFAULT 'STARTING',
    started_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    ready_at TIMESTAMPTZ,
    allocated_at TIMESTAMPTZ,
    ended_at TIMESTAMPTZ,
    last_heartbeat_at TIMESTAMPTZ,
    exit_code INTEGER,
    crash_reason TEXT,
    log_path VARCHAR(500),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS game_server_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    server_id UUID NOT NULL REFERENCES game_server_instances(id) ON DELETE CASCADE,
    event_type VARCHAR(100) NOT NULL,
    payload_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS port_allocations (
    port INTEGER PRIMARY KEY,
    status VARCHAR(50) NOT NULL DEFAULT 'FREE',
    server_id UUID REFERENCES game_server_instances(id),
    allocated_at TIMESTAMPTZ,
    released_at TIMESTAMPTZ
);

-- Settlement
CREATE TABLE IF NOT EXISTS match_results (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    session_id UUID NOT NULL,
    server_id UUID NOT NULL,
    mode VARCHAR(50) NOT NULL,
    map_id VARCHAR(100) NOT NULL,
    duration_seconds INTEGER NOT NULL,
    result_json JSONB NOT NULL DEFAULT '{}',
    idempotency_key VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS match_player_results (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    match_result_id UUID NOT NULL REFERENCES match_results(id) ON DELETE CASCADE,
    player_id UUID NOT NULL,
    team VARCHAR(50),
    result VARCHAR(50) NOT NULL,
    kills INTEGER NOT NULL DEFAULT 0,
    deaths INTEGER NOT NULL DEFAULT 0,
    assists INTEGER NOT NULL DEFAULT 0,
    score INTEGER NOT NULL DEFAULT 0,
    exp_delta BIGINT NOT NULL DEFAULT 0,
    reward_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Inventory
CREATE TABLE IF NOT EXISTS inventory_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL,
    item_id VARCHAR(255) NOT NULL,
    quantity BIGINT NOT NULL DEFAULT 1,
    expires_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(player_id, item_id)
);

CREATE TABLE IF NOT EXISTS inventory_logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL,
    item_id VARCHAR(255) NOT NULL,
    quantity_delta BIGINT NOT NULL,
    quantity_before BIGINT NOT NULL,
    quantity_after BIGINT NOT NULL,
    reason VARCHAR(100) NOT NULL,
    biz_type VARCHAR(50),
    biz_id VARCHAR(255),
    operator_id UUID,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Admin
CREATE TABLE IF NOT EXISTS admin_users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    username VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL DEFAULT 'GM',
    status VARCHAR(50) NOT NULL DEFAULT 'ACTIVE',
    failed_login_count INTEGER NOT NULL DEFAULT 0,
    locked_until TIMESTAMPTZ,
    last_login_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Support
CREATE TABLE IF NOT EXISTS crash_reports (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID,
    client_version VARCHAR(50),
    platform VARCHAR(50),
    crash_type VARCHAR(100),
    title VARCHAR(500),
    description TEXT,
    dump_url VARCHAR(500),
    log_url VARCHAR(500),
    metadata_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS player_feedback (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID,
    nickname VARCHAR(100),
    email VARCHAR(255),
    feedback_type VARCHAR(50) NOT NULL,
    title VARCHAR(500),
    content TEXT NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'OPEN',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ,
    handled_by UUID,
    handled_at TIMESTAMPTZ,
    handle_note TEXT
);

CREATE TABLE IF NOT EXISTS order_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL,
    platform VARCHAR(50) NOT NULL,
    platform_order_id VARCHAR(255),
    status VARCHAR(50) NOT NULL DEFAULT 'CREATED',
    amount BIGINT NOT NULL,
    currency VARCHAR(20) NOT NULL DEFAULT 'USD',
    item_json JSONB NOT NULL DEFAULT '{}',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    paid_at TIMESTAMPTZ,
    completed_at TIMESTAMPTZ,
    updated_at TIMESTAMPTZ
);

CREATE TABLE IF NOT EXISTS wallet_balances (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL UNIQUE,
    currency_type VARCHAR(50) NOT NULL,
    balance BIGINT NOT NULL DEFAULT 0,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS wallet_ledger (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID NOT NULL,
    currency_type VARCHAR(50) NOT NULL,
    amount BIGINT NOT NULL,
    balance_before BIGINT NOT NULL,
    balance_after BIGINT NOT NULL,
    biz_type VARCHAR(50) NOT NULL,
    biz_id VARCHAR(255) NOT NULL,
    idempotency_key VARCHAR(255) NOT NULL UNIQUE,
    operator_id UUID,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Indexes
CREATE INDEX IF NOT EXISTS idx_accounts_email ON accounts(email);
CREATE INDEX IF NOT EXISTS idx_accounts_status ON accounts(status);
CREATE INDEX IF NOT EXISTS idx_player_identities_account_id ON player_identities(account_id);
CREATE INDEX IF NOT EXISTS idx_refresh_tokens_account_id ON refresh_tokens(account_id);
CREATE INDEX IF NOT EXISTS idx_refresh_tokens_token_hash ON refresh_tokens(token_hash);
CREATE INDEX IF NOT EXISTS idx_ban_records_account_id ON ban_records(account_id);
CREATE INDEX IF NOT EXISTS idx_player_profiles_nickname ON player_profiles(nickname);
CREATE INDEX IF NOT EXISTS idx_player_profiles_status ON player_profiles(status);
CREATE INDEX IF NOT EXISTS idx_player_unlocks_player_id ON player_unlocks(player_id);
CREATE INDEX IF NOT EXISTS idx_player_event_logs_player_id ON player_event_logs(player_id);
CREATE INDEX IF NOT EXISTS idx_game_configs_key ON game_configs(config_key);
CREATE INDEX IF NOT EXISTS idx_game_configs_status ON game_configs(status);
CREATE INDEX IF NOT EXISTS idx_game_rooms_status ON game_rooms(status);
CREATE INDEX IF NOT EXISTS idx_game_rooms_mode ON game_rooms(mode);
CREATE INDEX IF NOT EXISTS idx_game_room_players_room_id ON game_room_players(room_id);
CREATE INDEX IF NOT EXISTS idx_game_room_players_player_id ON game_room_players(player_id);
CREATE INDEX IF NOT EXISTS idx_matchmaking_tickets_status ON matchmaking_tickets(status);
CREATE INDEX IF NOT EXISTS idx_matchmaking_tickets_player_id ON matchmaking_tickets(player_id);
CREATE INDEX IF NOT EXISTS idx_game_sessions_status ON game_sessions(status);
CREATE INDEX IF NOT EXISTS idx_game_sessions_source_id ON game_sessions(source_id);
CREATE INDEX IF NOT EXISTS idx_player_sessions_session_id ON player_sessions(game_session_id);
CREATE INDEX IF NOT EXISTS idx_player_sessions_player_id ON player_sessions(player_id);
CREATE INDEX IF NOT EXISTS idx_session_events_session_id ON session_events(game_session_id);
CREATE INDEX IF NOT EXISTS idx_game_server_instances_status ON game_server_instances(status);
CREATE INDEX IF NOT EXISTS idx_game_server_instances_session_id ON game_server_instances(session_id);
CREATE INDEX IF NOT EXISTS idx_match_results_session_id ON match_results(session_id);
CREATE INDEX IF NOT EXISTS idx_match_player_results_match_id ON match_player_results(match_result_id);
CREATE INDEX IF NOT EXISTS idx_inventory_items_player_id ON inventory_items(player_id);
CREATE INDEX IF NOT EXISTS idx_inventory_logs_player_id ON inventory_logs(player_id);
CREATE INDEX IF NOT EXISTS idx_admin_audit_logs_admin_user_id ON admin_audit_logs(admin_user_id);
CREATE INDEX IF NOT EXISTS idx_admin_audit_logs_created_at ON admin_audit_logs(created_at);
CREATE INDEX IF NOT EXISTS idx_crash_reports_player_id ON crash_reports(player_id);
CREATE INDEX IF NOT EXISTS idx_crash_reports_created_at ON crash_reports(created_at);
CREATE INDEX IF NOT EXISTS idx_player_feedback_status ON player_feedback(status);
CREATE INDEX IF NOT EXISTS idx_player_feedback_created_at ON player_feedback(created_at);
CREATE INDEX IF NOT EXISTS idx_wallet_balances_player_id ON wallet_balances(player_id);
CREATE INDEX IF NOT EXISTS idx_wallet_ledger_player_id ON wallet_ledger(player_id);