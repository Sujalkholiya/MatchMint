-- MatchMint SQLite3 Database Schema
PRAGMA foreign_keys = ON;

-- 1. Users table (Admins, Organizers, Referees, Captains, Spectators)
CREATE TABLE IF NOT EXISTS users (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('SUPER_ADMIN', 'ORGANIZER', 'REFEREE', 'CAPTAIN', 'SPECTATOR')),
    avatar_url TEXT,
    campus TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Competitions table (Tournaments)
CREATE TABLE IF NOT EXISTS competitions (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    category TEXT NOT NULL CHECK(category IN ('sports', 'esports')),
    format TEXT NOT NULL CHECK(format IN ('single_elimination', 'round_robin')),
    status TEXT NOT NULL DEFAULT 'upcoming' CHECK(status IN ('upcoming', 'live', 'completed')),
    description TEXT,
    total_teams INTEGER NOT NULL,
    registered_teams_count INTEGER NOT NULL DEFAULT 0,
    current_round INTEGER NOT NULL DEFAULT 0,
    total_rounds INTEGER NOT NULL,
    venues TEXT NOT NULL, -- JSON array of strings, e.g. ["Main Stadium Turf", "East Arena Field 2"]
    match_duration_minutes INTEGER NOT NULL DEFAULT 60,
    start_date TEXT NOT NULL,
    end_date TEXT NOT NULL,
    points_for_win INTEGER NOT NULL DEFAULT 3,
    points_for_draw INTEGER NOT NULL DEFAULT 1,
    points_for_loss INTEGER NOT NULL DEFAULT 0,
    created_by TEXT REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. Teams table (Squads)
CREATE TABLE IF NOT EXISTS teams (
    id TEXT PRIMARY KEY,
    competition_id TEXT NOT NULL REFERENCES competitions(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    short_tag TEXT NOT NULL,
    seed INTEGER NOT NULL,
    captain TEXT NOT NULL,
    email TEXT NOT NULL,
    roster_count INTEGER NOT NULL DEFAULT 5,
    wins INTEGER NOT NULL DEFAULT 0,
    draws INTEGER NOT NULL DEFAULT 0,
    losses INTEGER NOT NULL DEFAULT 0,
    points INTEGER NOT NULL DEFAULT 0,
    score_for INTEGER NOT NULL DEFAULT 0,
    score_against INTEGER NOT NULL DEFAULT 0,
    score_diff INTEGER NOT NULL DEFAULT 0,
    rank INTEGER NOT NULL DEFAULT 0,
    status TEXT NOT NULL DEFAULT 'active' CHECK(status IN ('active', 'eliminated')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 4. Matches table (Fixtures)
CREATE TABLE IF NOT EXISTS matches (
    id TEXT PRIMARY KEY,
    competition_id TEXT NOT NULL REFERENCES competitions(id) ON DELETE CASCADE,
    round INTEGER NOT NULL,
    round_name TEXT NOT NULL,
    match_number INTEGER NOT NULL,
    team1_id TEXT REFERENCES teams(id) ON DELETE SET NULL,
    team2_id TEXT REFERENCES teams(id) ON DELETE SET NULL,
    team1_score INTEGER,
    team2_score INTEGER,
    winner_id TEXT REFERENCES teams(id) ON DELETE SET NULL,
    status TEXT NOT NULL DEFAULT 'upcoming' CHECK(status IN ('upcoming', 'completed')),
    scheduled_time TEXT NOT NULL,
    venue TEXT NOT NULL,
    next_match_id TEXT REFERENCES matches(id) ON DELETE SET NULL,
    next_match_slot INTEGER CHECK(next_match_slot IN (1, 2)),
    tie_breaker_details TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 5. History / Audit Records table (One-Click Rollback Trail)
CREATE TABLE IF NOT EXISTS history_records (
    id TEXT PRIMARY KEY,
    competition_id TEXT NOT NULL REFERENCES competitions(id) ON DELETE CASCADE,
    match_id TEXT REFERENCES matches(id) ON DELETE SET NULL,
    action_type TEXT NOT NULL CHECK(action_type IN ('RESULT_ENTERED', 'RESULT_REVERTED', 'FIXTURES_GENERATED')),
    description TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    previous_state TEXT, -- JSON snapshot: {team1Score, team2Score, winnerId, status}
    can_undo INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 6. Indexes for performance
CREATE INDEX IF NOT EXISTS idx_competitions_status ON competitions(status);
CREATE INDEX IF NOT EXISTS idx_competitions_category ON competitions(category);
CREATE INDEX IF NOT EXISTS idx_teams_competition ON teams(competition_id);
CREATE INDEX IF NOT EXISTS idx_matches_competition ON matches(competition_id);
CREATE INDEX IF NOT EXISTS idx_matches_status ON matches(status);
CREATE INDEX IF NOT EXISTS idx_matches_scheduled ON matches(scheduled_time);
CREATE INDEX IF NOT EXISTS idx_history_competition ON history_records(competition_id);
