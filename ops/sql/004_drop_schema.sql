-- 004_drop_schema.sql
-- Drop all tables in reverse dependency order
-- Execution: ONLY when complete schema teardown is needed
-- WARNING: This will delete ALL data from all tables

-- ============================================================================
-- EXECUTION ORDER (reverse dependency order)
-- Leaf tables first (no dependents), then tables that depend on them, etc.
-- This ordering respects foreign key constraints to avoid violations
-- ============================================================================

-- PHASE 1: Leaf tables with no dependents (order within phase does not matter)
DROP TABLE IF EXISTS wallet_ledger;
DROP TABLE IF EXISTS inventory_logs;
DROP TABLE IF EXISTS session_events;
DROP TABLE IF EXISTS game_server_events;
DROP TABLE IF EXISTS port_allocations;
DROP TABLE IF EXISTS player_sessions;
DROP TABLE IF EXISTS match_player_results;
DROP TABLE IF EXISTS game_room_players;
DROP TABLE IF EXISTS player_identities;
DROP TABLE IF EXISTS refresh_tokens;
DROP TABLE IF EXISTS device_logins;
DROP TABLE IF EXISTS ban_records;
DROP TABLE IF EXISTS player_unlocks;
DROP TABLE IF EXISTS player_event_logs;
DROP TABLE IF EXISTS player_settings;
DROP TABLE IF EXISTS player_statistics;
DROP TABLE IF EXISTS player_feedback;
DROP TABLE IF EXISTS crash_reports;
DROP TABLE IF EXISTS order_records;
DROP TABLE IF EXISTS inventory_items;
DROP TABLE IF EXISTS wallet_balances;
DROP TABLE IF EXISTS match_results;

-- PHASE 2: Intermediate tables (depend on phase 1 tables)
DROP TABLE IF EXISTS game_sessions;
DROP TABLE IF EXISTS game_config_publish_logs;
DROP TABLE IF EXISTS admin_audit_logs;
DROP TABLE IF EXISTS game_configs;
DROP TABLE IF EXISTS game_server_instances;
DROP TABLE IF EXISTS game_rooms;
DROP TABLE IF EXISTS admin_users;
DROP TABLE IF EXISTS player_profiles;

-- PHASE 3: Root tables (top of dependency tree, everything depends on these)
DROP TABLE IF EXISTS accounts;

-- ============================================================================
-- SUMMARY: Table drop order explanation
-- ============================================================================
-- wallet_ledger (no dependents)
-- inventory_logs (no dependents)
-- session_events (no dependents)
-- game_server_events (no dependents)
-- port_allocations (no dependents)
-- player_sessions (depends on game_sessions)
-- match_player_results (depends on match_results)
-- game_room_players (depends on game_rooms)
-- player_identities (depends on accounts)
-- refresh_tokens (depends on accounts)
-- device_logins (depends on accounts)
-- ban_records (depends on accounts)
-- player_unlocks (depends on player_profiles)
-- player_event_logs (depends on player_profiles)
-- player_settings (depends on player_profiles)
-- player_statistics (depends on player_profiles)
-- player_feedback (depends on nothing)
-- crash_reports (depends on nothing)
-- order_records (depends on nothing)
-- inventory_items (depends on player_profiles)
-- wallet_balances (depends on player_profiles)
-- match_results (depends on game_sessions)
-- game_sessions (depends on game_server_instances)
-- game_config_publish_logs (depends on game_configs)
-- admin_audit_logs (depends on nothing)
-- game_configs (depends on nothing)
-- game_server_instances (depends on nothing)
-- game_rooms (depends on nothing)
-- admin_users (depends on nothing)
-- player_profiles (depends on player_identities via player_id)
-- accounts (root - everything above that has FKs to it)