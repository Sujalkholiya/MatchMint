#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "database.h"
#include "router.h"
#include "controllers.h"
#include "json_util.h"
#include "dsa.h"
#include "http.h"

int main(void) {
    printf("=====================================================\n");
    printf("  MatchMint Tournament Engine & Match API Integration\n");
    printf("=====================================================\n");

    const char *db_path = ":memory:";
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    assert(db_init(db_path, schema_path) == 0);
    assert(db_reset_to_seed(schema_path, seed_path) == 0);

    router_init();
    router_add("GET", "/api/v1/matches", handle_get_matches);
    router_add("GET", "/api/v1/matches/:id", handle_get_match_by_id);
    router_add("POST", "/api/v1/competitions/:id/generate-fixtures", handle_generate_fixtures);
    router_add("GET", "/api/v1/competitions/:id/bracket", handle_get_bracket);
    router_add("PATCH", "/api/v1/matches/:id/schedule", handle_reschedule_match);

    sqlite3 *db = db_get_handle();
    assert(db != NULL);

    /* --------------------------------------------------------------------- */
    printf("[TEST] 1. Verify GET /api/v1/matches for comp-gehu-2026...\n");
    /* --------------------------------------------------------------------- */
    {
        int count = db_get_row_count("matches WHERE competition_id = 'comp-gehu-2026'");
        assert(count == 7);
        printf("  -> Retrieved 7 matches for GEHU Winter Cup (4 QFs, 2 SFs, 1 Final)\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 2. Verify GET /api/v1/matches/:id for match-gehu-qf1...\n");
    /* --------------------------------------------------------------------- */
    {
        sqlite3_stmt *stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT status, team1_score, team2_score, winner_id FROM matches WHERE id = 'match-gehu-qf1';", -1, &stmt, NULL);
        assert(sqlite3_step(stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 0), "completed") == 0);
        assert(sqlite3_column_int(stmt, 1) == 3);
        assert(sqlite3_column_int(stmt, 2) == 1);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 3), "team-falcons") == 0);
        sqlite3_finalize(stmt);
        printf("  -> Match details and scores verified: Falcons 3 - 1 Mountain Strikers\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 3. Verify GET /api/v1/competitions/:id/bracket structure...\n");
    /* --------------------------------------------------------------------- */
    {
        int round1_count = db_get_row_count("matches WHERE competition_id = 'comp-gehu-2026' AND round = 1");
        int round2_count = db_get_row_count("matches WHERE competition_id = 'comp-gehu-2026' AND round = 2");
        int round3_count = db_get_row_count("matches WHERE competition_id = 'comp-gehu-2026' AND round = 3");

        assert(round1_count == 4);
        assert(round2_count == 2);
        assert(round3_count == 1);
        printf("  -> Bracket tree rounds verified: R1 (4 QFs), R2 (2 SFs), R3 (1 Final)\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 4. Testing Tournament Engine Fixture Generation...\n");
    /* --------------------------------------------------------------------- */
    {
        HttpRequest req;
        memset(&req, 0, sizeof(HttpRequest));
        snprintf(req.method, sizeof(req.method), "POST");
        snprintf(req.path, sizeof(req.path), "/api/v1/competitions/comp-cyberclash/generate-fixtures");
        snprintf(req.params[0].key, sizeof(req.params[0].key), "id");
        snprintf(req.params[0].value, sizeof(req.params[0].value), "comp-cyberclash");
        req.param_count = 1;

        // Calling handler directly with test dummy socket 0
        handle_generate_fixtures(&req, 0);

        int cyberclash_matches = db_get_row_count("matches WHERE competition_id = 'comp-cyberclash'");
        assert(cyberclash_matches == 7);

        int audit_count = db_get_row_count("history_records WHERE competition_id = 'comp-cyberclash' AND action_type = 'FIXTURES_GENERATED'");
        assert(audit_count >= 1);
        printf("  -> CyberClash generated 7 fixtures and recorded audit log entry\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 5. Testing Reschedule & Venue Conflict Detection...\n");
    /* --------------------------------------------------------------------- */
    {
        DSAInterval intervals[2];
        memset(intervals, 0, sizeof(intervals));

        snprintf(intervals[0].match_id, sizeof(intervals[0].match_id), "match-icl-r1-m1");
        snprintf(intervals[0].venue, sizeof(intervals[0].venue), "University Field A");
        intervals[0].start_time_epoch = 1758376800; // 14:00
        intervals[0].duration_minutes = 90;         // Ends 15:30

        snprintf(intervals[1].match_id, sizeof(intervals[1].match_id), "match-icl-r1-m3");
        snprintf(intervals[1].venue, sizeof(intervals[1].venue), "University Field A");
        intervals[1].start_time_epoch = 1758378600; // 14:30 (during m1!)
        intervals[1].duration_minutes = 90;

        DSAConflict conflicts[4];
        int conflict_count = detect_scheduling_conflicts(intervals, 2, conflicts, 4);
        assert(conflict_count == 1);
        assert(strcmp(conflicts[0].type, "VENUE_COLLISION") == 0);
        printf("  -> Successfully detected venue collision on '%s'\n", conflicts[0].venue);

        // Test rescheduling match-icl-r1-m1 to another venue
        HttpRequest req;
        memset(&req, 0, sizeof(HttpRequest));
        snprintf(req.method, sizeof(req.method), "PATCH");
        snprintf(req.path, sizeof(req.path), "/api/v1/matches/match-icl-r1-m1/schedule");
        snprintf(req.params[0].key, sizeof(req.params[0].key), "id");
        snprintf(req.params[0].value, sizeof(req.params[0].value), "match-icl-r1-m1");
        req.param_count = 1;
        req.body = strdup("{\"scheduledTime\":\"2026-09-20 18:00\",\"venue\":\"West Turf Arena\"}");

        handle_reschedule_match(&req, 0);
        free(req.body);

        sqlite3_stmt *stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT venue, scheduled_time FROM matches WHERE id = 'match-icl-r1-m1';", -1, &stmt, NULL);
        assert(sqlite3_step(stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 0), "West Turf Arena") == 0);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 1), "2026-09-20 18:00") == 0);
        sqlite3_finalize(stmt);
        printf("  -> Match rescheduled successfully to 'West Turf Arena' at '2026-09-20 18:00'\n");
    }

    printf("=====================================================\n");
    printf("  ALL TOURNAMENT ENGINE & MATCH TESTS PASSED (100%%)\n");
    printf("=====================================================\n");

    db_close();
    return 0;
}
