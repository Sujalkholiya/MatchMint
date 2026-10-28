-- MatchMint Initial Collegiate Seed Data
PRAGMA foreign_keys = ON;

-- Clean existing data
DELETE FROM history_records;
DELETE FROM matches;
DELETE FROM teams;
DELETE FROM competitions;
DELETE FROM users;

-- 1. Default Organizer User
INSERT INTO users (id, name, email, password_hash, role, campus) VALUES
('usr-admin', 'Campus Admin', 'admin@campus.edu', '$2b$12$K8y40BqUfD9jG2bW8ZgS5uWJtPq50V5.uE3Yl9U2C1pU6I3a7Zg6.', 'ORGANIZER', 'Graphic Era Hill University');

-- 2. Competitions
INSERT INTO competitions (id, name, category, format, status, description, total_teams, registered_teams_count, current_round, total_rounds, venues, match_duration_minutes, start_date, end_date, points_for_win, points_for_draw, points_for_loss, created_by) VALUES
('comp-gehu-2026', 'GEHU Winter Cup 2026', 'sports', 'single_elimination', 'live', 'Inter-department 7v7 football championship with 8 collegiate squads competing for the winter trophy.', 8, 8, 2, 3, '["Main Stadium Turf", "East Arena Field 2"]', 60, '2026-09-24', '2026-09-28', 3, 1, 0, 'usr-admin'),
('comp-cyberclash', 'CyberClash Masters', 'esports', 'single_elimination', 'upcoming', 'Tactical 5v5 FPS LAN tournament with 8 campus squads competing for the championship.', 8, 8, 1, 3, '["LAN Arena Pod 1", "LAN Arena Pod 2"]', 75, '2026-10-05', '2026-10-08', 3, 0, 0, 'usr-admin'),
('comp-inter-league', 'Inter-College League', 'sports', 'round_robin', 'live', 'Round-robin university league where every team plays each opponent across 5 game weeks.', 6, 6, 3, 5, '["University Field A", "University Field B"]', 50, '2026-09-20', '2026-10-04', 3, 1, 0, 'usr-admin');

-- 3. Teams (GEHU Winter Cup 2026 - 8 teams)
INSERT INTO teams (id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status) VALUES
('team-falcons', 'comp-gehu-2026', 'Falcons', 'FLC', 1, 'Aarav Sharma', 'aarav.falcons@campus.edu', 11, 2, 0, 0, 6, 5, 1, 4, 1, 'active'),
('team-code-warriors', 'comp-gehu-2026', 'Code Warriors', 'CDW', 2, 'Devansh Verma', 'devansh.cdw@campus.edu', 10, 2, 0, 0, 6, 4, 0, 4, 2, 'active'),
('team-mountain-strikers', 'comp-gehu-2026', 'Mountain Strikers', 'MTS', 3, 'Aditya Rawat', 'aditya.mts@campus.edu', 11, 1, 0, 1, 3, 3, 3, 0, 3, 'eliminated'),
('team-tech-titans', 'comp-gehu-2026', 'Tech Titans', 'TTN', 4, 'Rohan Mehta', 'rohan.ttn@campus.edu', 9, 1, 0, 1, 3, 2, 2, 0, 4, 'eliminated'),
('team-pixel-united', 'comp-gehu-2026', 'Pixel United', 'PXU', 5, 'Kunal Joshi', 'kunal.pxu@campus.edu', 10, 1, 0, 1, 3, 3, 4, -1, 5, 'active'),
('team-the-mavericks', 'comp-gehu-2026', 'The Mavericks', 'MVR', 6, 'Siddharth Negi', 'sid.mvr@campus.edu', 10, 0, 0, 1, 0, 1, 2, -1, 6, 'eliminated'),
('team-byte-force', 'comp-gehu-2026', 'Byte Force', 'BYF', 7, 'Varun Bhatt', 'varun.byf@campus.edu', 10, 0, 0, 1, 0, 0, 2, -2, 7, 'active'),
('team-north-stars', 'comp-gehu-2026', 'North Stars', 'NST', 8, 'Tanmay Bisht', 'tanmay.nst@campus.edu', 9, 0, 0, 1, 0, 1, 3, -2, 8, 'active');

-- Teams (CyberClash Masters - 8 teams)
INSERT INTO teams (id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status) VALUES
('team-apex-predators', 'comp-cyberclash', 'Apex Predators', 'APX', 1, 'Rishi Sen', 'rishi.apx@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 1, 'active'),
('team-phantom-strike', 'comp-cyberclash', 'Phantom Strike', 'PST', 2, 'Aryan Rawat', 'aryan.pst@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 2, 'active'),
('team-quantum-rush', 'comp-cyberclash', 'Quantum Rush', 'QTR', 3, 'Yash Mehra', 'yash.qtr@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 3, 'active'),
('team-shadow-syndicate', 'comp-cyberclash', 'Shadow Syndicate', 'SDS', 4, 'Karan Gusain', 'karan.sds@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 4, 'active'),
('team-vortex-ops', 'comp-cyberclash', 'Vortex Ops', 'VTX', 5, 'Sahil Negi', 'sahil.vtx@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 5, 'active'),
('team-blaze-legion', 'comp-cyberclash', 'Blaze Legion', 'BLZ', 6, 'Dhruv Singh', 'dhruv.blz@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 6, 'active'),
('team-nexus-protocol', 'comp-cyberclash', 'Nexus Protocol', 'NXP', 7, 'Ayush Bhatt', 'ayush.nxp@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 7, 'active'),
('team-iron-clad', 'comp-cyberclash', 'Iron Clad', 'IRN', 8, 'Manish Bisht', 'manish.irn@campus.edu', 5, 0, 0, 0, 0, 0, 0, 0, 8, 'active');

-- Teams (Inter-College League - 6 teams)
INSERT INTO teams (id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status) VALUES
('team-hill-riders', 'comp-inter-league', 'Hill Riders', 'HLR', 1, 'Prakash Pant', 'prakash.hlr@campus.edu', 11, 2, 1, 0, 7, 6, 2, 4, 1, 'active'),
('team-doon-tigers', 'comp-inter-league', 'Doon Tigers', 'DNT', 2, 'Rahul Negi', 'rahul.dnt@campus.edu', 11, 2, 0, 1, 6, 5, 3, 2, 2, 'active'),
('team-himalaya', 'comp-inter-league', 'Himalaya United', 'HMU', 3, 'Vivek Chauhan', 'vivek.hmu@campus.edu', 10, 1, 1, 1, 4, 4, 4, 0, 3, 'active'),
('team-valley', 'comp-inter-league', 'Valley Warriors', 'VLW', 4, 'Shivam Gusain', 'shivam.vlw@campus.edu', 10, 1, 0, 2, 3, 3, 5, -2, 4, 'active'),
('team-forest', 'comp-inter-league', 'Forest FC', 'FFC', 5, 'Nitin Pandey', 'nitin.ffc@campus.edu', 11, 1, 0, 2, 3, 2, 4, -2, 5, 'active'),
('team-river', 'comp-inter-league', 'River Strikers', 'RVS', 6, 'Gaurav Arya', 'gaurav.rvs@campus.edu', 10, 0, 2, 1, 2, 2, 4, -2, 6, 'active');

-- 4. Matches (GEHU Winter Cup 2026 Knockout Tree)
-- Finals
INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue, next_match_id, next_match_slot) VALUES
('match-gehu-f', 'comp-gehu-2026', 3, 'Final', 7, 'team-falcons', NULL, NULL, NULL, NULL, 'upcoming', '2026-09-28 17:00', 'Main Stadium Turf', NULL, NULL);

-- Semifinals
INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue, next_match_id, next_match_slot) VALUES
('match-gehu-sf1', 'comp-gehu-2026', 2, 'Semi Final 01', 5, 'team-falcons', 'team-code-warriors', 2, 0, 'team-falcons', 'completed', '2026-09-26 15:00', 'Main Stadium Turf', 'match-gehu-f', 1),
('match-gehu-sf2', 'comp-gehu-2026', 2, 'Semi Final 02', 6, 'team-pixel-united', NULL, NULL, NULL, NULL, 'upcoming', '2026-09-26 16:30', 'East Arena Field 2', 'match-gehu-f', 2);

-- Quarterfinals
INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue, next_match_id, next_match_slot) VALUES
('match-gehu-qf1', 'comp-gehu-2026', 1, 'Quarter Final 01', 1, 'team-falcons', 'team-mountain-strikers', 3, 1, 'team-falcons', 'completed', '2026-09-24 14:00', 'Main Stadium Turf', 'match-gehu-sf1', 1),
('match-gehu-qf2', 'comp-gehu-2026', 1, 'Quarter Final 02', 2, 'team-code-warriors', 'team-tech-titans', 2, 0, 'team-code-warriors', 'completed', '2026-09-24 15:15', 'East Arena Field 2', 'match-gehu-sf1', 2),
('match-gehu-qf3', 'comp-gehu-2026', 1, 'Quarter Final 03', 3, 'team-pixel-united', 'team-the-mavericks', 2, 1, 'team-pixel-united', 'completed', '2026-09-24 16:30', 'Main Stadium Turf', 'match-gehu-sf2', 1),
('match-gehu-qf4', 'comp-gehu-2026', 1, 'Quarter Final 04', 4, 'team-byte-force', 'team-north-stars', NULL, NULL, NULL, 'upcoming', '2026-09-25 18:45', 'East Arena Field 2', 'match-gehu-sf2', 2);

-- Matches (CyberClash Masters)
INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue, next_match_id, next_match_slot) VALUES
('match-cc-f', 'comp-cyberclash', 3, 'Grand Final', 7, NULL, NULL, NULL, NULL, NULL, 'upcoming', '2026-10-08 18:00', 'LAN Arena Pod 1', NULL, NULL),
('match-cc-sf1', 'comp-cyberclash', 2, 'Semi Final 1', 5, NULL, NULL, NULL, NULL, NULL, 'upcoming', '2026-10-07 14:00', 'LAN Arena Pod 1', 'match-cc-f', 1),
('match-cc-sf2', 'comp-cyberclash', 2, 'Semi Final 2', 6, NULL, NULL, NULL, NULL, NULL, 'upcoming', '2026-10-07 16:00', 'LAN Arena Pod 2', 'match-cc-f', 2),
('match-cc-qf1', 'comp-cyberclash', 1, 'Quarter Final 1', 1, 'team-apex-predators', 'team-iron-clad', NULL, NULL, NULL, 'upcoming', '2026-10-05 11:00', 'LAN Arena Pod 1', 'match-cc-sf1', 1),
('match-cc-qf2', 'comp-cyberclash', 1, 'Quarter Final 2', 2, 'team-phantom-strike', 'team-nexus-protocol', NULL, NULL, NULL, 'upcoming', '2026-10-05 13:00', 'LAN Arena Pod 2', 'match-cc-sf1', 2),
('match-cc-qf3', 'comp-cyberclash', 1, 'Quarter Final 3', 3, 'team-quantum-rush', 'team-blaze-legion', NULL, NULL, NULL, 'upcoming', '2026-10-05 15:00', 'LAN Arena Pod 1', 'match-cc-sf2', 1),
('match-cc-qf4', 'comp-cyberclash', 1, 'Quarter Final 4', 4, 'team-shadow-syndicate', 'team-vortex-ops', NULL, NULL, NULL, 'upcoming', '2026-10-05 17:00', 'LAN Arena Pod 2', 'match-cc-sf2', 2);

-- Matches (Inter-College League)
INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue) VALUES
('match-icl-r1-m1', 'comp-inter-league', 1, 'Round 1 Match 1', 1, 'team-hill-riders', 'team-forest', 2, 0, 'team-hill-riders', 'completed', '2026-09-20 14:00', 'University Field A'),
('match-icl-r1-m2', 'comp-inter-league', 1, 'Round 1 Match 2', 2, 'team-doon-tigers', 'team-river', 2, 0, 'team-doon-tigers', 'completed', '2026-09-20 15:30', 'University Field B'),
('match-icl-r1-m3', 'comp-inter-league', 1, 'Round 1 Match 3', 3, 'team-himalaya', 'team-valley', 1, 0, 'team-himalaya', 'completed', '2026-09-20 17:00', 'University Field A'),
('match-icl-r2-m1', 'comp-inter-league', 2, 'Round 2 Match 1', 4, 'team-hill-riders', 'team-doon-tigers', 2, 1, 'team-hill-riders', 'completed', '2026-09-24 14:00', 'University Field A'),
('match-icl-r2-m2', 'comp-inter-league', 2, 'Round 2 Match 2', 5, 'team-river', 'team-himalaya', 1, 1, NULL, 'completed', '2026-09-24 15:30', 'University Field B'),
('match-icl-r2-m3', 'comp-inter-league', 2, 'Round 2 Match 3', 6, 'team-valley', 'team-forest', 2, 1, 'team-valley', 'completed', '2026-09-24 17:00', 'University Field A'),
('match-icl-r3-m1', 'comp-inter-league', 3, 'Round 3 Match 1', 7, 'team-hill-riders', 'team-himalaya', 2, 2, NULL, 'completed', '2026-09-28 14:00', 'University Field A'),
('match-icl-r3-m2', 'comp-inter-league', 3, 'Round 3 Match 2', 8, 'team-doon-tigers', 'team-valley', 2, 1, 'team-doon-tigers', 'completed', '2026-09-28 15:30', 'University Field B'),
('match-icl-r3-m3', 'comp-inter-league', 3, 'Round 3 Match 3', 9, 'team-forest', 'team-river', 1, 1, NULL, 'completed', '2026-09-28 17:00', 'University Field A');

-- 5. History / Audit Log Records
INSERT INTO history_records (id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo) VALUES
('hist-001', 'comp-gehu-2026', 'match-gehu-qf1', 'RESULT_ENTERED', 'Falcons defeated Mountain Strikers (3 — 1) in Quarter Final 01', 'Today · 10:42 AM', '{"team1Score":null,"team2Score":null,"winnerId":null,"status":"upcoming"}', 1),
('hist-002', 'comp-gehu-2026', NULL, 'FIXTURES_GENERATED', 'Generated Quarter Final fixtures for 8 teams', 'Today · 10:18 AM', NULL, 0),
('hist-003', 'comp-gehu-2026', 'match-gehu-qf2', 'RESULT_ENTERED', 'Code Warriors defeated Tech Titans (2 — 0) in Quarter Final 02', 'Yesterday · 4:15 PM', '{"team1Score":null,"team2Score":null,"winnerId":null,"status":"upcoming"}', 0),
('hist-004', 'comp-inter-league', 'match-icl-r3-m2', 'RESULT_ENTERED', 'Doon Tigers defeated Valley Warriors (2 — 1) in Round 3', 'Yesterday · 2:30 PM', '{"team1Score":null,"team2Score":null,"winnerId":null,"status":"upcoming"}', 0);
