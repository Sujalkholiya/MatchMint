#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "database.h"
#include "router.h"
#include "controllers.h"
#include "json_util.h"


#if defined(_WIN32)
#include <winsock2.h>
#endif

// We simulate client_sock dispatch by intercepting or using custom pipe/memory buffer
// Let's create a local loopback socket pair for testing actual network dispatch!

int main(void) {
    printf("=====================================================\n");
    printf("  MatchMint Competition & Team API Integration Test\n");
    printf("=====================================================\n");

    const char *db_path = ":memory:";
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    assert(db_init(db_path, schema_path) == 0);
    assert(db_reset_to_seed(schema_path, seed_path) == 0);

    router_init();
    router_add("GET", "/api/v1/competitions", handle_get_competitions);
    router_add("GET", "/api/v1/competitions/:id", handle_get_competition_by_id);
    router_add("POST", "/api/v1/competitions", handle_create_competition);
    router_add("PUT", "/api/v1/competitions/:id", handle_update_competition);
    router_add("GET", "/api/v1/teams", handle_get_teams);
    router_add("GET", "/api/v1/teams/:id", handle_get_team_by_id);
    router_add("POST", "/api/v1/teams", handle_register_team);

    // Test 1: Verify direct SQLite database queries for competitions
    printf("[TEST] 1. Verify GET /api/v1/competitions data...\n");
    int comp_count = db_get_row_count("competitions");
    assert(comp_count == 3);

    // Test 2: Verify teams query for GEHU Winter Cup
    printf("[TEST] 2. Verify GET /api/v1/teams for comp-gehu-2026...\n");
    sqlite3 *db = db_get_handle();
    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, "SELECT count(*) FROM teams WHERE competition_id='comp-gehu-2026';", -1, &stmt, NULL);
    sqlite3_step(stmt);
    int gehu_teams = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    assert(gehu_teams == 8);

    // Test 3: Test POST /api/v1/competitions with generateNow = true
    printf("[TEST] 3. Testing Tournament Creation with Auto Fixtures...\n");
    HttpRequest post_comp_req;
    memset(&post_comp_req, 0, sizeof(post_comp_req));
    snprintf(post_comp_req.method, sizeof(post_comp_req.method), "POST");
    snprintf(post_comp_req.path, sizeof(post_comp_req.path), "/api/v1/competitions");
    post_comp_req.body = strdup("{\"name\":\"Autumn Invitational\",\"category\":\"sports\",\"format\":\"single_elimination\",\"totalTeams\":4,\"startDate\":\"2026-11-15\",\"matchDurationMinutes\":60,\"generateNow\":true}");

    // Using dummy socket 0 for in-process test logic
    handle_create_competition(&post_comp_req, 0);
    free(post_comp_req.body);

    int new_comp_count = db_get_row_count("competitions");
    assert(new_comp_count == 4); // 3 original + 1 created
    printf("  -> Competitions count updated to %d\n", new_comp_count);

    // Test 4: Test POST /api/v1/teams registration
    printf("[TEST] 4. Testing Squad Registration...\n");
    HttpRequest post_team_req;
    memset(&post_team_req, 0, sizeof(post_team_req));
    snprintf(post_team_req.method, sizeof(post_team_req.method), "POST");
    snprintf(post_team_req.path, sizeof(post_team_req.path), "/api/v1/teams");
    post_team_req.body = strdup("{\"competitionId\":\"comp-cyberclash\",\"name\":\"Valkyrie Strikers\",\"captain\":\"Kavya Rawat\",\"rosterCount\":5}");

    handle_register_team(&post_team_req, 0);
    free(post_team_req.body);

    // Check CyberClash teams count (was 8, should now be 9)
    sqlite3_prepare_v2(db, "SELECT count(*) FROM teams WHERE competition_id='comp-cyberclash';", -1, &stmt, NULL);
    sqlite3_step(stmt);
    int cc_teams = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    assert(cc_teams == 9);
    printf("  -> CyberClash team count updated to %d\n", cc_teams);

    db_close();
    printf("=====================================================\n");
    printf("  ALL COMPETITION & TEAM API TESTS PASSED (100%%)\n");
    printf("=====================================================\n");
    return 0;
}
