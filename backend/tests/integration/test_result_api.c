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
    printf("  MatchMint Result Entry, Standings & Undo API Tests \n");
    printf("=====================================================\n");

    const char *db_path = ":memory:";
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    assert(db_init(db_path, schema_path) == 0);
    assert(db_reset_to_seed(schema_path, seed_path) == 0);

    router_init();
    router_add("POST", "/api/v1/matches/:id/result", handle_submit_match_result);
    router_add("GET", "/api/v1/competitions/:id/standings", handle_get_standings);
    router_add("GET", "/api/v1/competitions/:id/history", handle_get_history);
    router_add("POST", "/api/v1/history/:id/undo", handle_undo_history);

    sqlite3 *db = db_get_handle();
    assert(db != NULL);

    /* --------------------------------------------------------------------- */
    printf("[TEST] 1. Verify Standings for Inter-College League...\n");
    /* --------------------------------------------------------------------- */
    {
        sqlite3_stmt *stmt = NULL;
        const char *sql = "SELECT id, name, points, rank FROM teams WHERE competition_id = 'comp-inter-league' ORDER BY rank ASC;";
        assert(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK);

        assert(sqlite3_step(stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 0), "team-hill-riders") == 0);
        assert(sqlite3_column_int(stmt, 2) == 7); // 7 points
        assert(sqlite3_column_int(stmt, 3) == 1); // Rank 1

        assert(sqlite3_step(stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(stmt, 0), "team-doon-tigers") == 0);
        assert(sqlite3_column_int(stmt, 2) == 6); // 6 points
        assert(sqlite3_column_int(stmt, 3) == 2); // Rank 2
        sqlite3_finalize(stmt);

        printf("  -> Standings verified: Hill Riders (Rank 1, 7 pts), Doon Tigers (Rank 2, 6 pts)\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 2. Submit Match Result with Binary Tree Auto-Advancement...\n");
    /* --------------------------------------------------------------------- */
    char submitted_hist_id[64] = {0};
    {
        // match-cc-qf1 is Apex Predators vs Iron Clad. Next match is match-cc-sf1 slot 1.
        HttpRequest req;
        memset(&req, 0, sizeof(HttpRequest));
        snprintf(req.method, sizeof(req.method), "POST");
        snprintf(req.path, sizeof(req.path), "/api/v1/matches/match-cc-qf1/result");
        snprintf(req.params[0].key, sizeof(req.params[0].key), "id");
        snprintf(req.params[0].value, sizeof(req.params[0].value), "match-cc-qf1");
        req.param_count = 1;
        req.body = strdup("{\"team1Score\":2,\"team2Score\":0,\"tieBreakerDetails\":\"Clean sweep\"}");

        handle_submit_match_result(&req, 0);
        free(req.body);

        // Verify match status and winner
        sqlite3_stmt *m_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT status, winner_id, team1_score, team2_score FROM matches WHERE id = 'match-cc-qf1';", -1, &m_stmt, NULL);
        assert(sqlite3_step(m_stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(m_stmt, 0), "completed") == 0);
        assert(strcmp((const char *)sqlite3_column_text(m_stmt, 1), "team-apex-predators") == 0);
        assert(sqlite3_column_int(m_stmt, 2) == 2);
        assert(sqlite3_column_int(m_stmt, 3) == 0);
        sqlite3_finalize(m_stmt);

        // Verify winner auto-advanced to Semi Final 1 slot 1
        sqlite3_stmt *sf_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT team1_id FROM matches WHERE id = 'match-cc-sf1';", -1, &sf_stmt, NULL);
        assert(sqlite3_step(sf_stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(sf_stmt, 0), "team-apex-predators") == 0);
        sqlite3_finalize(sf_stmt);

        // Verify audit record created
        sqlite3_stmt *h_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT id, can_undo FROM history_records WHERE match_id = 'match-cc-qf1' AND can_undo = 1;", -1, &h_stmt, NULL);
        assert(sqlite3_step(h_stmt) == SQLITE_ROW);
        snprintf(submitted_hist_id, sizeof(submitted_hist_id), "%s", (const char *)sqlite3_column_text(h_stmt, 0));
        assert(sqlite3_column_int(h_stmt, 1) == 1);
        sqlite3_finalize(h_stmt);

        printf("  -> Apex Predators advanced to Semi Final 1 slot 1, audit ID: %s\n", submitted_hist_id);
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 3. Test One-Click Rollback (Reverting match-cc-qf1)...\n");
    /* --------------------------------------------------------------------- */
    {
        HttpRequest undo_req;
        memset(&undo_req, 0, sizeof(HttpRequest));
        snprintf(undo_req.method, sizeof(undo_req.method), "POST");
        snprintf(undo_req.path, sizeof(undo_req.path), "/api/v1/history/%s/undo", submitted_hist_id);
        snprintf(undo_req.params[0].key, sizeof(undo_req.params[0].key), "id");
        snprintf(undo_req.params[0].value, sizeof(undo_req.params[0].value), "%s", submitted_hist_id);
        undo_req.param_count = 1;

        handle_undo_history(&undo_req, 0);

        // Verify Semi Final 1 slot 1 has been cleared back to NULL
        sqlite3_stmt *sf_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT team1_id FROM matches WHERE id = 'match-cc-sf1';", -1, &sf_stmt, NULL);
        assert(sqlite3_step(sf_stmt) == SQLITE_ROW);
        assert(sqlite3_column_type(sf_stmt, 0) == SQLITE_NULL);
        sqlite3_finalize(sf_stmt);

        // Verify match-cc-qf1 reverted back to upcoming and scores cleared
        sqlite3_stmt *m_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT status, team1_score, team2_score, winner_id FROM matches WHERE id = 'match-cc-qf1';", -1, &m_stmt, NULL);
        assert(sqlite3_step(m_stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(m_stmt, 0), "upcoming") == 0);
        assert(sqlite3_column_type(m_stmt, 1) == SQLITE_NULL);
        assert(sqlite3_column_type(m_stmt, 2) == SQLITE_NULL);
        assert(sqlite3_column_type(m_stmt, 3) == SQLITE_NULL);
        sqlite3_finalize(m_stmt);

        // Verify history record can_undo is now 0
        sqlite3_stmt *h_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT can_undo FROM history_records WHERE id = ?;", -1, &h_stmt, NULL);
        sqlite3_bind_text(h_stmt, 1, submitted_hist_id, -1, SQLITE_STATIC);
        assert(sqlite3_step(h_stmt) == SQLITE_ROW);
        assert(sqlite3_column_int(h_stmt, 0) == 0);
        sqlite3_finalize(h_stmt);

        // Verify RESULT_REVERTED audit log added
        int revert_count = db_get_row_count("history_records WHERE match_id = 'match-cc-qf1' AND action_type = 'RESULT_REVERTED'");
        assert(revert_count >= 1);

        printf("  -> Successfully reverted match result; cleared advanced slot and marked history undone\n");
    }

    /* --------------------------------------------------------------------- */
    printf("[TEST] 4. Downstream Completion Rollback Safety Invariant...\n");
    /* --------------------------------------------------------------------- */
    {
        // 1. Re-enter QF1 (Apex wins -> advances to SF1 slot 1)
        HttpRequest qf1_req;
        memset(&qf1_req, 0, sizeof(HttpRequest));
        snprintf(qf1_req.method, sizeof(qf1_req.method), "POST");
        snprintf(qf1_req.path, sizeof(qf1_req.path), "/api/v1/matches/match-cc-qf1/result");
        snprintf(qf1_req.params[0].key, sizeof(qf1_req.params[0].key), "id");
        snprintf(qf1_req.params[0].value, sizeof(qf1_req.params[0].value), "match-cc-qf1");
        qf1_req.param_count = 1;
        qf1_req.body = strdup("{\"team1Score\":3,\"team2Score\":1}");
        handle_submit_match_result(&qf1_req, 0);
        free(qf1_req.body);

        // Get new history ID for QF1
        char new_qf1_hist[64];
        sqlite3_stmt *h_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT id FROM history_records WHERE match_id = 'match-cc-qf1' AND can_undo = 1;", -1, &h_stmt, NULL);
        assert(sqlite3_step(h_stmt) == SQLITE_ROW);
        snprintf(new_qf1_hist, sizeof(new_qf1_hist), "%s", (const char *)sqlite3_column_text(h_stmt, 0));
        sqlite3_finalize(h_stmt);

        // 2. Populate SF1 slot 2 with a team and mark SF1 as completed
        sqlite3_exec(db, "UPDATE matches SET team2_id = 'team-phantom-strike', team1_score = 2, team2_score = 1, winner_id = 'team-apex-predators', status = 'completed' WHERE id = 'match-cc-sf1';", NULL, NULL, NULL);

        // 3. Now attempt to rollback QF1!
        HttpRequest bad_undo;
        memset(&bad_undo, 0, sizeof(HttpRequest));
        snprintf(bad_undo.method, sizeof(bad_undo.method), "POST");
        snprintf(bad_undo.path, sizeof(bad_undo.path), "/api/v1/history/%s/undo", new_qf1_hist);
        snprintf(bad_undo.params[0].key, sizeof(bad_undo.params[0].key), "id");
        snprintf(bad_undo.params[0].value, sizeof(bad_undo.params[0].value), "%s", new_qf1_hist);
        bad_undo.param_count = 1;

        handle_undo_history(&bad_undo, 0);

        // QF1 must STILL be completed because downstream match SF1 is completed!
        sqlite3_stmt *chk_stmt = NULL;
        sqlite3_prepare_v2(db, "SELECT status FROM matches WHERE id = 'match-cc-qf1';", -1, &chk_stmt, NULL);
        assert(sqlite3_step(chk_stmt) == SQLITE_ROW);
        assert(strcmp((const char *)sqlite3_column_text(chk_stmt, 0), "completed") == 0);
        sqlite3_finalize(chk_stmt);

        printf("  -> Rollback safely rejected: downstream match is completed, preserving bracket tree integrity\n");
    }

    printf("=====================================================\n");
    printf("  ALL RESULT ENTRY, STANDINGS & UNDO TESTS PASSED (100%%)\n");
    printf("=====================================================\n");

    db_close();
    return 0;
}
