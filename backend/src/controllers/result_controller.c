#include "controllers.h"
#include "database.h"
#include "response.h"
#include "json_util.h"
#include "dsa.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_hist_seq = 100;

/* Helper to recalculate standings using C Merge Sort and persist to SQLite */
static int recalculate_and_save_standings(sqlite3 *db, const char *comp_id) {
    if (!db || !comp_id) return -1;

    // 1. Fetch points rules
    const char *c_sql = "SELECT points_for_win, points_for_draw, points_for_loss FROM competitions WHERE id = ?;";
    sqlite3_stmt *c_stmt = NULL;
    if (sqlite3_prepare_v2(db, c_sql, -1, &c_stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_text(c_stmt, 1, comp_id, -1, SQLITE_STATIC);
    int pts_win = 3, pts_draw = 1, pts_loss = 0;
    if (sqlite3_step(c_stmt) == SQLITE_ROW) {
        pts_win = sqlite3_column_int(c_stmt, 0);
        pts_draw = sqlite3_column_int(c_stmt, 1);
        pts_loss = sqlite3_column_int(c_stmt, 2);
    }
    sqlite3_finalize(c_stmt);

    // 2. Fetch participating teams
    const char *t_sql = "SELECT id, name, short_tag, seed, captain, roster_count, status FROM teams WHERE competition_id = ?;";
    sqlite3_stmt *t_stmt = NULL;
    if (sqlite3_prepare_v2(db, t_sql, -1, &t_stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_text(t_stmt, 1, comp_id, -1, SQLITE_STATIC);

    DSATeam teams[64];
    int team_count = 0;
    while (sqlite3_step(t_stmt) == SQLITE_ROW && team_count < 64) {
        DSATeam *t = &teams[team_count++];
        memset(t, 0, sizeof(DSATeam));
        snprintf(t->id, sizeof(t->id), "%s", (const char *)sqlite3_column_text(t_stmt, 0));
        snprintf(t->name, sizeof(t->name), "%s", (const char *)sqlite3_column_text(t_stmt, 1));
        snprintf(t->short_tag, sizeof(t->short_tag), "%s", (const char *)sqlite3_column_text(t_stmt, 2));
        t->seed = sqlite3_column_int(t_stmt, 3);
        snprintf(t->captain, sizeof(t->captain), "%s", (const char *)sqlite3_column_text(t_stmt, 4));
        t->roster_count = sqlite3_column_int(t_stmt, 5);
        snprintf(t->status, sizeof(t->status), "%s", (const char *)sqlite3_column_text(t_stmt, 6));
    }
    sqlite3_finalize(t_stmt);

    if (team_count == 0) return 0;

    // 3. Fetch all completed matches for this competition
    const char *m_sql = "SELECT team1_id, team2_id, team1_score, team2_score FROM matches WHERE competition_id = ? AND status = 'completed';";
    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_text(m_stmt, 1, comp_id, -1, SQLITE_STATIC);

    while (sqlite3_step(m_stmt) == SQLITE_ROW) {
        const char *t1_id = (const char *)sqlite3_column_text(m_stmt, 0);
        const char *t2_id = (const char *)sqlite3_column_text(m_stmt, 1);
        int s1 = sqlite3_column_int(m_stmt, 2);
        int s2 = sqlite3_column_int(m_stmt, 3);

        if (!t1_id || !t2_id) continue;

        DSATeam *t1 = NULL;
        DSATeam *t2 = NULL;
        for (int i = 0; i < team_count; i++) {
            if (strcmp(teams[i].id, t1_id) == 0) t1 = &teams[i];
            if (strcmp(teams[i].id, t2_id) == 0) t2 = &teams[i];
        }

        if (t1) {
            t1->score_for += s1;
            t1->score_against += s2;
            if (s1 > s2) {
                t1->wins += 1;
                t1->points += pts_win;
            } else if (s1 < s2) {
                t1->losses += 1;
                t1->points += pts_loss;
            } else {
                t1->draws += 1;
                t1->points += pts_draw;
            }
        }

        if (t2) {
            t2->score_for += s2;
            t2->score_against += s1;
            if (s2 > s1) {
                t2->wins += 1;
                t2->points += pts_win;
            } else if (s2 < s1) {
                t2->losses += 1;
                t2->points += pts_loss;
            } else {
                t2->draws += 1;
                t2->points += pts_draw;
            }
        }
    }
    sqlite3_finalize(m_stmt);

    for (int i = 0; i < team_count; i++) {
        teams[i].score_diff = teams[i].score_for - teams[i].score_against;
    }

    // 4. Stable Multi-Attribute Merge Sort
    merge_sort_standings(teams, team_count);

    // 5. Update team stats and ranks in database
    const char *u_sql = "UPDATE teams SET wins = ?, draws = ?, losses = ?, points = ?, score_for = ?, score_against = ?, score_diff = ?, rank = ? WHERE id = ?;";
    sqlite3_stmt *u_stmt = NULL;
    if (sqlite3_prepare_v2(db, u_sql, -1, &u_stmt, NULL) != SQLITE_OK) return -1;

    for (int i = 0; i < team_count; i++) {
        sqlite3_bind_int(u_stmt, 1, teams[i].wins);
        sqlite3_bind_int(u_stmt, 2, teams[i].draws);
        sqlite3_bind_int(u_stmt, 3, teams[i].losses);
        sqlite3_bind_int(u_stmt, 4, teams[i].points);
        sqlite3_bind_int(u_stmt, 5, teams[i].score_for);
        sqlite3_bind_int(u_stmt, 6, teams[i].score_against);
        sqlite3_bind_int(u_stmt, 7, teams[i].score_diff);
        sqlite3_bind_int(u_stmt, 8, teams[i].rank);
        sqlite3_bind_text(u_stmt, 9, teams[i].id, -1, SQLITE_STATIC);

        sqlite3_step(u_stmt);
        sqlite3_reset(u_stmt);
    }
    sqlite3_finalize(u_stmt);

    return 0;
}

/* POST /api/v1/matches/:id/result */
void handle_submit_match_result(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db || !req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing request body");
        return;
    }

    const char *match_id = req->params[0].value;
    int team1_score = 0;
    int team2_score = 0;
    char tie_breaker[128] = {0};

    if (!json_get_int(req->body, "team1Score", &team1_score) ||
        !json_get_int(req->body, "team2Score", &team2_score)) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Scores must be provided as numbers");
        return;
    }

    json_get_string(req->body, "tieBreakerDetails", tie_breaker, sizeof(tie_breaker));

    // 1. Fetch current match details
    const char *m_sql = "SELECT m.id, m.competition_id, m.team1_id, m.team2_id, m.team1_score, m.team2_score, "
                        "m.winner_id, m.status, m.next_match_id, m.next_match_slot, c.format, "
                        "t1.name, t2.name, m.round_name "
                        "FROM matches m "
                        "JOIN competitions c ON m.competition_id = c.id "
                        "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                        "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                        "WHERE m.id = ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, match_id, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Match not found");
        return;
    }

    char comp_id[64] = {0};
    char team1_id[64] = {0};
    char team2_id[64] = {0};
    char next_match_id[64] = {0};
    char format[32] = {0};
    char t1_name[64] = "Team 1";
    char t2_name[64] = "Team 2";
    char round_name[64] = {0};

    snprintf(comp_id, sizeof(comp_id), "%s", (const char *)sqlite3_column_text(stmt, 1));
    const char *p_t1 = (const char *)sqlite3_column_text(stmt, 2);
    if (p_t1) snprintf(team1_id, sizeof(team1_id), "%s", p_t1);
    const char *p_t2 = (const char *)sqlite3_column_text(stmt, 3);
    if (p_t2) snprintf(team2_id, sizeof(team2_id), "%s", p_t2);

    int prev_s1_type = sqlite3_column_type(stmt, 4);
    int prev_s2_type = sqlite3_column_type(stmt, 5);
    int prev_s1 = sqlite3_column_int(stmt, 4);
    int prev_s2 = sqlite3_column_int(stmt, 5);
    char prev_win_buf[64] = {0};
    char prev_stat_buf[32] = "upcoming";

    const char *pw = (const char *)sqlite3_column_text(stmt, 6);
    if (pw) snprintf(prev_win_buf, sizeof(prev_win_buf), "%s", pw);
    const char *ps = (const char *)sqlite3_column_text(stmt, 7);
    if (ps) snprintf(prev_stat_buf, sizeof(prev_stat_buf), "%s", ps);

    const char *p_nm = (const char *)sqlite3_column_text(stmt, 8);
    if (p_nm) snprintf(next_match_id, sizeof(next_match_id), "%s", p_nm);
    int next_match_slot = sqlite3_column_int(stmt, 9);
    snprintf(format, sizeof(format), "%s", (const char *)sqlite3_column_text(stmt, 10));

    const char *p_n1 = (const char *)sqlite3_column_text(stmt, 11);
    if (p_n1) snprintf(t1_name, sizeof(t1_name), "%s", p_n1);
    const char *p_n2 = (const char *)sqlite3_column_text(stmt, 12);
    if (p_n2) snprintf(t2_name, sizeof(t2_name), "%s", p_n2);
    const char *p_rn = (const char *)sqlite3_column_text(stmt, 13);
    if (p_rn) snprintf(round_name, sizeof(round_name), "%s", p_rn);

    sqlite3_finalize(stmt);

    // 2. Build JSON snapshot of previous state for one-click rollback
    char prev_state_json[256];
    char s1_buf[16] = "null";
    char s2_buf[16] = "null";
    char win_buf[72] = "null";

    if (prev_s1_type != SQLITE_NULL) snprintf(s1_buf, sizeof(s1_buf), "%d", prev_s1);
    if (prev_s2_type != SQLITE_NULL) snprintf(s2_buf, sizeof(s2_buf), "%d", prev_s2);
    if (strlen(prev_win_buf) > 0) snprintf(win_buf, sizeof(win_buf), "\"%s\"", prev_win_buf);

    snprintf(
        prev_state_json, sizeof(prev_state_json),
        "{\"team1Score\":%s,\"team2Score\":%s,\"winnerId\":%s,\"status\":\"%s\"}",
        s1_buf, s2_buf, win_buf, prev_stat_buf
    );

    // 3. Determine winner
    char winner_id[64] = {0};
    if (team1_score > team2_score) {
        snprintf(winner_id, sizeof(winner_id), "%s", team1_id);
    } else if (team2_score > team1_score) {
        snprintf(winner_id, sizeof(winner_id), "%s", team2_id);
    } else {
        if (strcmp(format, "single_elimination") == 0) {
            snprintf(winner_id, sizeof(winner_id), "%s", team1_id); // Default or tiebreaker
        }
    }

    // 4. Begin transaction
    db_begin_transaction();

    // Auto-advance winner in knockout binary tree
    if (strlen(next_match_id) > 0 && strlen(winner_id) > 0) {
        const char *adv_sql = (next_match_slot == 1)
            ? "UPDATE matches SET team1_id = ? WHERE id = ?;"
            : "UPDATE matches SET team2_id = ? WHERE id = ?;";

        sqlite3_stmt *adv_stmt = NULL;
        sqlite3_prepare_v2(db, adv_sql, -1, &adv_stmt, NULL);
        sqlite3_bind_text(adv_stmt, 1, winner_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(adv_stmt, 2, next_match_id, -1, SQLITE_STATIC);
        sqlite3_step(adv_stmt);
        sqlite3_finalize(adv_stmt);
    }

    // Update current match
    const char *upd_m = "UPDATE matches SET team1_score = ?, team2_score = ?, winner_id = ?, status = 'completed', tie_breaker_details = ? WHERE id = ?;";
    sqlite3_stmt *u_stmt = NULL;
    sqlite3_prepare_v2(db, upd_m, -1, &u_stmt, NULL);
    sqlite3_bind_int(u_stmt, 1, team1_score);
    sqlite3_bind_int(u_stmt, 2, team2_score);
    if (strlen(winner_id) > 0) sqlite3_bind_text(u_stmt, 3, winner_id, -1, SQLITE_STATIC);
    else sqlite3_bind_null(u_stmt, 3);

    if (strlen(tie_breaker) > 0) sqlite3_bind_text(u_stmt, 4, tie_breaker, -1, SQLITE_STATIC);
    else sqlite3_bind_null(u_stmt, 4);

    sqlite3_bind_text(u_stmt, 5, match_id, -1, SQLITE_STATIC);
    sqlite3_step(u_stmt);
    sqlite3_finalize(u_stmt);

    // Recalculate standings using C Merge Sort
    recalculate_and_save_standings(db, comp_id);

    // Invalidate previous history undo eligibility for this match
    const char *inv_sql = "UPDATE history_records SET can_undo = 0 WHERE match_id = ?;";
    sqlite3_stmt *inv_stmt = NULL;
    sqlite3_prepare_v2(db, inv_sql, -1, &inv_stmt, NULL);
    sqlite3_bind_text(inv_stmt, 1, match_id, -1, SQLITE_STATIC);
    sqlite3_step(inv_stmt);
    sqlite3_finalize(inv_stmt);

    // Insert new audit log entry
    char hist_id[64];
    snprintf(hist_id, sizeof(hist_id), "hist-%lld-%d", (long long)time(NULL), ++g_hist_seq);

    char desc[256];
    if (strlen(winner_id) > 0) {
        const char *w_name = (strcmp(winner_id, team1_id) == 0) ? t1_name : t2_name;
        snprintf(desc, sizeof(desc), "%s defeated %s (%d — %d) in %s",
                 w_name, (strcmp(winner_id, team1_id) == 0) ? t2_name : t1_name,
                 team1_score, team2_score, round_name);
    } else {
        snprintf(desc, sizeof(desc), "%s drew against %s (%d — %d) in %s",
                 t1_name, t2_name, team1_score, team2_score, round_name);
    }

    const char *h_sql = "INSERT INTO history_records (id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo) "
                        "VALUES (?, ?, ?, 'RESULT_ENTERED', ?, datetime('now'), ?, 1);";
    sqlite3_stmt *h_stmt = NULL;
    sqlite3_prepare_v2(db, h_sql, -1, &h_stmt, NULL);
    sqlite3_bind_text(h_stmt, 1, hist_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 2, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 3, match_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 4, desc, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 5, prev_state_json, -1, SQLITE_STATIC);
    sqlite3_step(h_stmt);
    sqlite3_finalize(h_stmt);

    db_commit_transaction();

    // 5. Send response
    char msg[256];
    snprintf(msg, sizeof(msg), "Result recorded: %s %d — %d %s", t1_name, team1_score, team2_score, t2_name);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append_printf(
        jb,
        "{\"matchId\":\"%s\",\"team1Score\":%d,\"team2Score\":%d,\"winnerId\":%s%s%s}",
        match_id, team1_score, team2_score,
        strlen(winner_id) > 0 ? "\"" : "",
        strlen(winner_id) > 0 ? winner_id : "null",
        strlen(winner_id) > 0 ? "\"" : ""
    );

    response_send_success(client_sock, 200, msg, jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/competitions/:id/standings */
void handle_get_standings(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    const char *comp_id = req->params[0].value;

    const char *sql = "SELECT id, competition_id, name, short_tag, seed, captain, email, "
                      "roster_count, wins, draws, losses, points, score_for, score_against, "
                      "score_diff, rank, status FROM teams "
                      "WHERE competition_id = ? "
                      "ORDER BY rank ASC, seed ASC;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, comp_id, -1, SQLITE_STATIC);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 2);
        const char *tag = (const char *)sqlite3_column_text(stmt, 3);
        int seed = sqlite3_column_int(stmt, 4);
        const char *captain = (const char *)sqlite3_column_text(stmt, 5);
        const char *email = (const char *)sqlite3_column_text(stmt, 6);
        int roster = sqlite3_column_int(stmt, 7);
        int wins = sqlite3_column_int(stmt, 8);
        int draws = sqlite3_column_int(stmt, 9);
        int losses = sqlite3_column_int(stmt, 10);
        int points = sqlite3_column_int(stmt, 11);
        int score_for = sqlite3_column_int(stmt, 12);
        int score_against = sqlite3_column_int(stmt, 13);
        int score_diff = sqlite3_column_int(stmt, 14);
        int rank = sqlite3_column_int(stmt, 15);
        const char *status = (const char *)sqlite3_column_text(stmt, 16);

        char esc_name[128];
        char esc_captain[128];
        json_escape_string(name ? name : "", esc_name, sizeof(esc_name));
        json_escape_string(captain ? captain : "", esc_captain, sizeof(esc_captain));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d,"
            "\"captain\":\"%s\",\"email\":\"%s\",\"rosterCount\":%d,\"wins\":%d,\"draws\":%d,\"losses\":%d,"
            "\"points\":%d,\"scoreFor\":%d,\"scoreAgainst\":%d,\"scoreDiff\":%d,\"rank\":%d,\"status\":\"%s\"}",
            id ? id : "",
            comp_id,
            esc_name,
            tag ? tag : "",
            seed,
            esc_captain,
            email ? email : "",
            roster,
            wins, draws, losses, points, score_for, score_against, score_diff, rank,
            status ? status : "active"
        );
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Standings retrieved", jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/competitions/:id/history and GET /api/v1/history */
void handle_get_history(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    char comp_id[64] = {0};
    if (req->param_count > 0 && strcmp(req->params[0].key, "id") == 0) {
        snprintf(comp_id, sizeof(comp_id), "%s", req->params[0].value);
    } else {
        http_get_query_param(req->query, "competitionId", comp_id, sizeof(comp_id));
    }

    char sql[512] = "SELECT id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo "
                    "FROM history_records ";
    if (strlen(comp_id) > 0) {
        strcat(sql, "WHERE competition_id = ? ");
    }
    strcat(sql, "ORDER BY rowid DESC LIMIT 50;");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    if (strlen(comp_id) > 0) {
        sqlite3_bind_text(stmt, 1, comp_id, -1, SQLITE_STATIC);
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *h_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *c_id = (const char *)sqlite3_column_text(stmt, 1);
        const char *m_id = (const char *)sqlite3_column_text(stmt, 2);
        const char *action = (const char *)sqlite3_column_text(stmt, 3);
        const char *desc = (const char *)sqlite3_column_text(stmt, 4);
        const char *time_str = (const char *)sqlite3_column_text(stmt, 5);
        int can_undo = sqlite3_column_int(stmt, 7);

        char esc_desc[256];
        json_escape_string(desc ? desc : "", esc_desc, sizeof(esc_desc));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",",
            h_id ? h_id : "",
            c_id ? c_id : ""
        );

        if (m_id) json_buffer_append_printf(jb, "\"matchId\":\"%s\",", m_id);
        else json_buffer_append(jb, "\"matchId\":null,");

        json_buffer_append_printf(
            jb,
            "\"actionType\":\"%s\",\"description\":\"%s\",\"timestamp\":\"%s\",\"canUndo\":%s}",
            action ? action : "",
            esc_desc,
            time_str ? time_str : "",
            can_undo ? "true" : "false"
        );
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "History records retrieved", jb->data);
    json_buffer_free(jb);
}

/* POST /api/v1/history/:historyId/undo */
void handle_undo_history(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    const char *history_id = req->params[0].value;

    // 1. Fetch history record
    const char *h_sql = "SELECT id, competition_id, match_id, action_type, description, previous_state, can_undo "
                        "FROM history_records WHERE id = ?;";
    sqlite3_stmt *h_stmt = NULL;
    if (sqlite3_prepare_v2(db, h_sql, -1, &h_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(h_stmt, 1, history_id, -1, SQLITE_STATIC);
    if (sqlite3_step(h_stmt) != SQLITE_ROW) {
        sqlite3_finalize(h_stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "History record not found");
        return;
    }

    char comp_id[64] = {0};
    char match_id[64] = {0};
    char prev_state[256] = {0};

    snprintf(comp_id, sizeof(comp_id), "%s", (const char *)sqlite3_column_text(h_stmt, 1));
    const char *m_id = (const char *)sqlite3_column_text(h_stmt, 2);
    if (m_id) snprintf(match_id, sizeof(match_id), "%s", m_id);

    const char *p_state = (const char *)sqlite3_column_text(h_stmt, 5);
    if (p_state) snprintf(prev_state, sizeof(prev_state), "%s", p_state);
    int can_undo = sqlite3_column_int(h_stmt, 6);
    sqlite3_finalize(h_stmt);

    if (!can_undo || strlen(match_id) == 0 || strlen(prev_state) == 0) {
        response_send_error(client_sock, 400, "CANNOT_UNDO", "This result cannot be undone.");
        return;
    }

    // 2. Fetch target match details
    const char *m_sql = "SELECT id, next_match_id, next_match_slot, winner_id, round_name FROM matches WHERE id = ?;";
    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(m_stmt, 1, match_id, -1, SQLITE_STATIC);
    if (sqlite3_step(m_stmt) != SQLITE_ROW) {
        sqlite3_finalize(m_stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Target match not found");
        return;
    }

    char next_match_id[64] = {0};
    char current_winner_id[64] = {0};
    char round_name[64] = "Match";

    const char *p_nm = (const char *)sqlite3_column_text(m_stmt, 1);
    if (p_nm) snprintf(next_match_id, sizeof(next_match_id), "%s", p_nm);
    int next_match_slot = sqlite3_column_int(m_stmt, 2);
    const char *p_cw = (const char *)sqlite3_column_text(m_stmt, 3);
    if (p_cw) snprintf(current_winner_id, sizeof(current_winner_id), "%s", p_cw);
    const char *p_rn = (const char *)sqlite3_column_text(m_stmt, 4);
    if (p_rn) snprintf(round_name, sizeof(round_name), "%s", p_rn);
    sqlite3_finalize(m_stmt);

    // 3. Downstream verification: Check if next round match has already completed
    if (strlen(next_match_id) > 0) {
        const char *nm_sql = "SELECT status FROM matches WHERE id = ?;";
        sqlite3_stmt *nm_stmt = NULL;
        sqlite3_prepare_v2(db, nm_sql, -1, &nm_stmt, NULL);
        sqlite3_bind_text(nm_stmt, 1, next_match_id, -1, SQLITE_STATIC);

        if (sqlite3_step(nm_stmt) == SQLITE_ROW) {
            const char *nm_status = (const char *)sqlite3_column_text(nm_stmt, 0);
            if (nm_status && strcmp(nm_status, "completed") == 0) {
                sqlite3_finalize(nm_stmt);
                response_send_error(client_sock, 409, "DOWNSTREAM_COMPLETED", "Cannot undo: Next round match has already been played.");
                return;
            }
        }
        sqlite3_finalize(nm_stmt);
    }

    // 4. Begin transaction for atomic rollback
    db_begin_transaction();

    // Clear advanced slot in next match
    if (strlen(next_match_id) > 0 && strlen(current_winner_id) > 0) {
        const char *clr_sql = (next_match_slot == 1)
            ? "UPDATE matches SET team1_id = NULL WHERE id = ? AND team1_id = ?;"
            : "UPDATE matches SET team2_id = NULL WHERE id = ? AND team2_id = ?;";

        sqlite3_stmt *clr_stmt = NULL;
        sqlite3_prepare_v2(db, clr_sql, -1, &clr_stmt, NULL);
        sqlite3_bind_text(clr_stmt, 1, next_match_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(clr_stmt, 2, current_winner_id, -1, SQLITE_STATIC);
        sqlite3_step(clr_stmt);
        sqlite3_finalize(clr_stmt);
    }

    // Revert target match to previous state
    int prev_t1 = 0, prev_t2 = 0;
    bool has_s1 = json_get_int(prev_state, "team1Score", &prev_t1);
    bool has_s2 = json_get_int(prev_state, "team2Score", &prev_t2);
    char prev_winner[64] = {0};
    bool has_winner = json_get_string(prev_state, "winnerId", prev_winner, sizeof(prev_winner));
    char prev_status[32] = "upcoming";
    json_get_string(prev_state, "status", prev_status, sizeof(prev_status));

    const char *rev_sql = "UPDATE matches SET team1_score = ?, team2_score = ?, winner_id = ?, status = ?, tie_breaker_details = NULL WHERE id = ?;";
    sqlite3_stmt *rev_stmt = NULL;
    sqlite3_prepare_v2(db, rev_sql, -1, &rev_stmt, NULL);

    if (has_s1) sqlite3_bind_int(rev_stmt, 1, prev_t1);
    else sqlite3_bind_null(rev_stmt, 1);

    if (has_s2) sqlite3_bind_int(rev_stmt, 2, prev_t2);
    else sqlite3_bind_null(rev_stmt, 2);

    if (has_winner && strlen(prev_winner) > 0) sqlite3_bind_text(rev_stmt, 3, prev_winner, -1, SQLITE_STATIC);
    else sqlite3_bind_null(rev_stmt, 3);

    sqlite3_bind_text(rev_stmt, 4, prev_status, -1, SQLITE_STATIC);
    sqlite3_bind_text(rev_stmt, 5, match_id, -1, SQLITE_STATIC);
    sqlite3_step(rev_stmt);
    sqlite3_finalize(rev_stmt);

    // Recalculate standings using C Merge Sort
    recalculate_and_save_standings(db, comp_id);

    // Invalidate undo on current history record
    const char *dis_sql = "UPDATE history_records SET can_undo = 0 WHERE id = ?;";
    sqlite3_stmt *dis_stmt = NULL;
    sqlite3_prepare_v2(db, dis_sql, -1, &dis_stmt, NULL);
    sqlite3_bind_text(dis_stmt, 1, history_id, -1, SQLITE_STATIC);
    sqlite3_step(dis_stmt);
    sqlite3_finalize(dis_stmt);

    // Add revert history entry
    char new_hist_id[64];
    snprintf(new_hist_id, sizeof(new_hist_id), "hist-revert-%lld-%d", (long long)time(NULL), ++g_hist_seq);
    char desc[256];
    snprintf(desc, sizeof(desc), "Reverted result for %s", round_name);

    const char *add_h = "INSERT INTO history_records (id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo) "
                        "VALUES (?, ?, ?, 'RESULT_REVERTED', ?, datetime('now'), NULL, 0);";
    sqlite3_stmt *add_stmt = NULL;
    sqlite3_prepare_v2(db, add_h, -1, &add_stmt, NULL);
    sqlite3_bind_text(add_stmt, 1, new_hist_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(add_stmt, 2, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(add_stmt, 3, match_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(add_stmt, 4, desc, -1, SQLITE_STATIC);
    sqlite3_step(add_stmt);
    sqlite3_finalize(add_stmt);

    db_commit_transaction();

    response_send_success(client_sock, 200, "Result successfully undone.", "{\"success\":true}");
}
