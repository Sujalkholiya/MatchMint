#include "common.h"
#include "database.h"
#include "server.h"
#include "router.h"
#include "response.h"
#include "json_util.h"
#include "controllers.h"

static void handle_health_check(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    response_send_success(client_sock, 200, "MatchMint API is healthy and operational", "{\"status\":\"healthy\",\"version\":\"" MATCHMINT_VERSION "\",\"service\":\"MatchMint C REST API\"}");
}

int main(int argc, char **argv) {
    printf("=====================================================\n");
    printf("  MatchMint C Backend Engine v%s\n", MATCHMINT_VERSION);
    printf("  Sports & Esports Tournament Management System\n");
    printf("=====================================================\n");

#if defined(__STDC_VERSION__)
    printf("[INFO] C Standard Version: %ld\n", __STDC_VERSION__);
#endif
    printf("[INFO] SQLite3 Library Version: %s\n", sqlite3_libversion());

    const char *db_path = "backend/database/matchmint.db";
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    /* Initialize database with schema */
    printf("[INFO] Initializing persistent database at '%s'...\n", db_path);
    if (db_init(db_path, schema_path) != 0) {
        fprintf(stderr, "[FATAL] Failed to initialize SQLite database.\n");
        return 1;
    }

    /* Auto-seed if database is brand new */
    if (db_get_row_count("competitions") == 0) {
        printf("[INFO] Applying collegiate seed dataset from '%s'...\n", seed_path);
        if (db_reset_to_seed(schema_path, seed_path) != 0) {
            fprintf(stderr, "[FATAL] Failed to apply seed dataset.\n");
            db_close();
            return 1;
        }
    }

    /* Initialize Router & Register Endpoints */
    router_init();
    router_add("GET", "/api/v1/health", handle_health_check);

    /* Competition Routes */
    router_add("GET", "/api/v1/competitions", handle_get_competitions);
    router_add("GET", "/api/v1/competitions/:id", handle_get_competition_by_id);
    router_add("POST", "/api/v1/competitions", handle_create_competition);
    router_add("PUT", "/api/v1/competitions/:id", handle_update_competition);

    /* Team Routes */
    router_add("GET", "/api/v1/teams", handle_get_teams);
    router_add("GET", "/api/v1/teams/:id", handle_get_team_by_id);
    router_add("POST", "/api/v1/teams", handle_register_team);

    /* Match & Bracket Routes */
    router_add("GET", "/api/v1/matches", handle_get_matches);
    router_add("GET", "/api/v1/matches/:id", handle_get_match_by_id);
    router_add("POST", "/api/v1/competitions/:id/generate-fixtures", handle_generate_fixtures);
    router_add("GET", "/api/v1/competitions/:id/bracket", handle_get_bracket);
    router_add("PATCH", "/api/v1/matches/:id/schedule", handle_reschedule_match);

    /* Result, Standings & Rollback Routes */
    router_add("POST", "/api/v1/matches/:id/result", handle_submit_match_result);
    router_add("GET", "/api/v1/competitions/:id/standings", handle_get_standings);
    router_add("GET", "/api/v1/competitions/:id/history", handle_get_history);
    router_add("GET", "/api/v1/history", handle_get_history);
    router_add("POST", "/api/v1/history/:id/undo", handle_undo_history);

    /* Dashboard, Venue & Settings Routes */
    router_add("GET", "/api/v1/dashboard/metrics", handle_get_dashboard_metrics);
    router_add("GET", "/api/v1/dashboard/today-matches", handle_get_today_matches);
    router_add("GET", "/api/v1/venues", handle_get_venues);
    router_add("POST", "/api/v1/competitions/:id/check-conflicts", handle_check_conflicts);
    router_add("GET", "/api/v1/settings/backup/export", handle_export_backup);
    router_add("POST", "/api/v1/settings/backup/import", handle_import_backup);
    router_add("POST", "/api/v1/settings/reset", handle_reset_database);

    printf("[INFO] Registered 25 REST endpoints (Health, Competitions, Teams, Fixtures, Brackets, Matches, Results, Standings, History/Undo, Dashboard, Venues, Backup/Reset).\n");

    /* Initialize HTTP Server on Port 8080 */
    int port = MATCHMINT_DEFAULT_PORT;
    if (server_init(port) != 0) {
        fprintf(stderr, "[FATAL] Failed to bind HTTP server to port %d.\n", port);
        db_close();
        return 1;
    }

    /* Check if single-pass test mode is requested */
    if (argc > 1 && strcmp(argv[1], "--test-init") == 0) {
        printf("[INFO] Test initialization mode completed successfully.\n");
        server_close();
        db_close();
        return 0;
    }

    printf("[INFO] Server ready! Waiting for client requests at http://localhost:%d/api/v1 ...\n", port);
    printf("[INFO] (Press Ctrl+C to stop)\n");

    /* Main event loop */
    server_start_loop(NULL);

    server_close();
    db_close();
    return 0;
}
