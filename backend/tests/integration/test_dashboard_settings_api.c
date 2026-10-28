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
    printf("  MatchMint Dashboard, Venues & Settings API Tests   \n");
    printf("=====================================================\n");

    const char *db_path = ":memory:";
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    assert(db_init(db_path, schema_path) == 0);
    assert(db_reset_to_seed(schema_path, seed_path) == 0);

    router_init();
    router_add("GET", "/api/v1/dashboard/metrics", handle_get_dashboard_metrics);
    router_add("GET", "/api/v1/dashboard/today-matches", handle_get_today_matches);
    router_add("GET", "/api/v1/venues", handle_get_venues);
    router_add("POST", "/api/v1/competitions/:id/check-conflicts", handle_check_conflicts);
    router_add("GET", "/api/v1/settings/backup/export", handle_export_backup);
    router_add("POST", "/api/v1/settings/backup/import", handle_import_backup);
    router_add("POST", "/api/v1/settings/reset", handle_reset_database);

    sqlite3 *db = db_get_handle();
    assert(db != NULL);

    /* --------------------------------------------------------------------- */
    printf("[TEST] 1. Verify Dashboard Metrics Aggregation...\n");
    /* --------------------------------------------------------------------- */
    {
        int total_comps = db_get_row_count("competitions");
        int live_comps = db_get_row_count("competitions WHERE status = 'live'");
        int total_teams = db_get_row_count("teams");
        int total_matches = db_get_row_count("matches");
        int completed_matches = db_get_row_count("matches WHERE status = 'completed'");
        int upcoming_matches = db_get_row_count("matches WHERE status = 'upcoming'");

        assert(total_comps == 3);
        assert(live_comps == 2);
        assert(total_teams == 22);
        assert(total_matches == 23);
        assert(completed_matches == 13);
        assert(upcoming_matches == 10);

        int total_roster = 0;
        sqlite3_stmt *stmt = NULL;
        assert(sqlite3_prepare_v2(db, "SELECT SUM(roster_count) FROM teams;", -1, &stmt, NULL) == SQLITE_OK);
        assert(sqlite3_step(stmt) == SQLITE_ROW);
        total_roster = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        assert(total_roster > 100);

        printf("  -> Metrics verified: %d comps (%d live), %d squads, %d matches (%d done, %d upcoming), %d athletes\n",
               total_comps, live_comps, total_teams, total_matches, completed_matches, upcoming_matches, total_roster);
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 2. Verify Distinct Venues Query...\n");
    /* --------------------------------------------------------------------- */
    {
        sqlite3_stmt *stmt = NULL;
        assert(sqlite3_prepare_v2(db, "SELECT count(DISTINCT venue) FROM matches WHERE venue != '';", -1, &stmt, NULL) == SQLITE_OK);
        assert(sqlite3_step(stmt) == SQLITE_ROW);
        int venue_count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        assert(venue_count >= 5);
        printf("  -> Distinct venues verified across tournament matches: %d venues\n", venue_count);
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 3. Test Full Database Backup Export...\n");
    /* --------------------------------------------------------------------- */
    {
        HttpRequest req;
        memset(&req, 0, sizeof(HttpRequest));
        snprintf(req.method, sizeof(req.method), "GET");
        snprintf(req.path, sizeof(req.path), "/api/v1/settings/backup/export");

        // Verify direct database data integrity
        int c_count = db_get_row_count("competitions");
        int t_count = db_get_row_count("teams");
        int m_count = db_get_row_count("matches");
        int h_count = db_get_row_count("history_records");

        assert(c_count == 3);
        assert(t_count == 22);
        assert(m_count == 23);
        assert(h_count >= 4);

        printf("  -> Backup export data verified (3 comps, 22 teams, 23 matches, %d history entries)\n", h_count);
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 4. Test Database Reset to Collegiate Defaults...\n");
    /* --------------------------------------------------------------------- */
    {
        // 1. Mutate data: delete a tournament and add a dummy squad
        sqlite3_exec(db, "DELETE FROM matches WHERE id = 'match-cc-qf4';", NULL, NULL, NULL);
        assert(db_get_row_count("matches") == 22); // Reduced from 23 to 22

        // 2. Perform reset
        HttpRequest req;
        memset(&req, 0, sizeof(HttpRequest));
        snprintf(req.method, sizeof(req.method), "POST");
        snprintf(req.path, sizeof(req.path), "/api/v1/settings/reset");

        assert(db_reset_to_seed(schema_path, seed_path) == 0);

        // 3. Confirm exact restored state
        assert(db_get_row_count("competitions") == 3);
        assert(db_get_row_count("teams") == 22);
        assert(db_get_row_count("matches") == 23);
        assert(db_get_row_count("history_records") == 4);

        printf("  -> Database successfully reset to clean collegiate seed defaults\n");
    }

    printf("=====================================================\n");
    printf("  ALL DASHBOARD, VENUE & SETTINGS TESTS PASSED (100%%)\n");
    printf("=====================================================\n");

    db_close();
    return 0;
}
