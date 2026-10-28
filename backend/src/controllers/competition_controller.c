#include "controllers.h"
#include "database.h"
#include "response.h"
#include "json_util.h"
#include "dsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static void serialize_competition_row(sqlite3_stmt *stmt, JsonBuffer *jb) {
    const char *id = (const char *)sqlite3_column_text(stmt, 0);
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    const char *category = (const char *)sqlite3_column_text(stmt, 2);
    const char *format = (const char *)sqlite3_column_text(stmt, 3);
    const char *status = (const char *)sqlite3_column_text(stmt, 4);
    const char *description = (const char *)sqlite3_column_text(stmt, 5);
    int total_teams = sqlite3_column_int(stmt, 6);
    int reg_teams = sqlite3_column_int(stmt, 7);
    int curr_round = sqlite3_column_int(stmt, 8);
    int total_rounds = sqlite3_column_int(stmt, 9);
    const char *venues = (const char *)sqlite3_column_text(stmt, 10);
    int duration = sqlite3_column_int(stmt, 11);
    const char *start_date = (const char *)sqlite3_column_text(stmt, 12);
    const char *end_date = (const char *)sqlite3_column_text(stmt, 13);
    int win_pts = sqlite3_column_int(stmt, 14);
    int draw_pts = sqlite3_column_int(stmt, 15);
    int loss_pts = sqlite3_column_int(stmt, 16);

    char esc_name[256];
    char esc_desc[512];
    json_escape_string(name ? name : "", esc_name, sizeof(esc_name));
    json_escape_string(description ? description : "", esc_desc, sizeof(esc_desc));

    json_buffer_append_printf(
        jb,
        "{\"id\":\"%s\",\"name\":\"%s\",\"category\":\"%s\",\"format\":\"%s\",\"status\":\"%s\","
        "\"description\":\"%s\",\"totalTeams\":%d,\"registeredTeamsCount\":%d,\"currentRound\":%d,"
        "\"totalRounds\":%d,\"venues\":%s,\"matchDurationMinutes\":%d,\"startDate\":\"%s\","
        "\"endDate\":\"%s\",\"pointsForWin\":%d,\"pointsForDraw\":%d,\"pointsForLoss\":%d}",
        id ? id : "",
        esc_name,
        category ? category : "sports",
        format ? format : "single_elimination",
        status ? status : "upcoming",
        esc_desc,
        total_teams,
        reg_teams,
        curr_round,
        total_rounds,
        (venues && venues[0] == '[') ? venues : "[\"Main Stadium\"]",
        duration,
        start_date ? start_date : "",
        end_date ? end_date : "",
        win_pts,
        draw_pts,
        loss_pts
    );
}

void handle_get_competitions(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database handle unavailable");
        return;
    }

    char cat_filter[32] = {0};
    char fmt_filter[32] = {0};
    char stat_filter[32] = {0};

    http_get_query_param(req->query, "category", cat_filter, sizeof(cat_filter));
    http_get_query_param(req->query, "format", fmt_filter, sizeof(fmt_filter));
    http_get_query_param(req->query, "status", stat_filter, sizeof(stat_filter));

    // Dynamic SQL query building with WHERE clauses
    char sql[512] = "SELECT id, name, category, format, status, description, total_teams, "
                    "registered_teams_count, current_round, total_rounds, venues, match_duration_minutes, "
                    "start_date, end_date, points_for_win, points_for_draw, points_for_loss "
                    "FROM competitions WHERE 1=1";

    if (cat_filter[0] && strcmp(cat_filter, "all") != 0) {
        strcat(sql, " AND category = @cat");
    }
    if (fmt_filter[0] && strcmp(fmt_filter, "all") != 0) {
        strcat(sql, " AND format = @fmt");
    }
    if (stat_filter[0] && strcmp(stat_filter, "all") != 0) {
        strcat(sql, " AND status = @stat");
    }
    strcat(sql, " ORDER BY created_at DESC;");

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "QUERY_ERROR", sqlite3_errmsg(db));
        return;
    }

    if (cat_filter[0] && strcmp(cat_filter, "all") != 0) {
        int idx = sqlite3_bind_parameter_index(stmt, "@cat");
        sqlite3_bind_text(stmt, idx, cat_filter, -1, SQLITE_STATIC);
    }
    if (fmt_filter[0] && strcmp(fmt_filter, "all") != 0) {
        int idx = sqlite3_bind_parameter_index(stmt, "@fmt");
        sqlite3_bind_text(stmt, idx, fmt_filter, -1, SQLITE_STATIC);
    }
    if (stat_filter[0] && strcmp(stat_filter, "all") != 0) {
        int idx = sqlite3_bind_parameter_index(stmt, "@stat");
        sqlite3_bind_text(stmt, idx, stat_filter, -1, SQLITE_STATIC);
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;
        serialize_competition_row(stmt, jb);
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Competitions retrieved successfully", jb->data);
    json_buffer_free(jb);
}

void handle_get_competition_by_id(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    const char *id = http_get_path_param(req, "id");
    if (!db || !id) {
        response_send_error(client_sock, 400, "INVALID_REQUEST", "Missing tournament ID");
        return;
    }

    const char *sql = "SELECT id, name, category, format, status, description, total_teams, "
                      "registered_teams_count, current_round, total_rounds, venues, match_duration_minutes, "
                      "start_date, end_date, points_for_win, points_for_draw, points_for_loss "
                      "FROM competitions WHERE id = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "QUERY_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        JsonBuffer *jb = json_buffer_create();
        serialize_competition_row(stmt, jb);
        sqlite3_finalize(stmt);

        response_send_success(client_sock, 200, "Competition details retrieved", jb->data);
        json_buffer_free(jb);
    } else {
        sqlite3_finalize(stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Competition not found");
    }
}

static int g_comp_seq = 100;

void handle_create_competition(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db || !req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing request body");
        return;
    }

    char name[128] = {0};
    char category[32] = "sports";
    char format[32] = "single_elimination";
    int total_teams = 8;
    char start_date[32] = "2026-10-30";
    int duration = 60;
    bool generate_now = false;

    json_get_string(req->body, "name", name, sizeof(name));
    json_get_string(req->body, "category", category, sizeof(category));
    json_get_string(req->body, "format", format, sizeof(format));
    json_get_int(req->body, "totalTeams", &total_teams);
    json_get_string(req->body, "startDate", start_date, sizeof(start_date));
    json_get_int(req->body, "matchDurationMinutes", &duration);
    json_get_bool(req->body, "generateNow", &generate_now);

    if (strlen(name) == 0) {
        response_send_error(client_sock, 400, "VALIDATION_ERROR", "Competition name is required");
        return;
    }

    char comp_id[64];
    snprintf(comp_id, sizeof(comp_id), "comp-%lld-%d", (long long)time(NULL), ++g_comp_seq);

    int rounds = (strcmp(format, "single_elimination") == 0)
        ? (int)ceil(log2((double)total_teams))
        : (total_teams - 1);

    char desc[256];
    snprintf(desc, sizeof(desc), "%s tournament. Format: %s.",
        strcmp(category, "sports") == 0 ? "Sports" : "Esports",
        strcmp(format, "round_robin") == 0 ? "Round Robin" : "Single Elimination"
    );

    db_begin_transaction();

    const char *sql = "INSERT INTO competitions (id, name, category, format, status, description, "
                      "total_teams, registered_teams_count, current_round, total_rounds, venues, "
                      "match_duration_minutes, start_date, end_date, points_for_win, points_for_draw, points_for_loss) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, 0, 0, ?, '[\"Main Stadium\", \"Arena 2\"]', ?, ?, ?, 3, 1, 0);";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        db_rollback_transaction();
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, format, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, generate_now ? "live" : "upcoming", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, desc, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, total_teams);
    sqlite3_bind_int(stmt, 8, rounds);
    sqlite3_bind_int(stmt, 9, duration);
    sqlite3_bind_text(stmt, 10, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, start_date, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        db_rollback_transaction();
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    // Auto-generate placeholder teams if generateNow is true
    if (generate_now) {
        DSATeam *teams = (DSATeam *)calloc(total_teams, sizeof(DSATeam));

        const char *team_sql = "INSERT INTO teams (id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status) "
                               "VALUES (?, ?, ?, ?, ?, ?, ?, ?, 0, 0, 0, 0, 0, 0, 0, ?, 'active');";

        sqlite3_stmt *t_stmt = NULL;
        sqlite3_prepare_v2(db, team_sql, -1, &t_stmt, NULL);

        for (int i = 0; i < total_teams; i++) {
            char t_id[64];
            char t_name[64];
            char t_tag[8];
            char t_capt[64];
            char t_email[64];

            snprintf(t_id, sizeof(t_id), "team-%.45s-%d", comp_id, i + 1);
            snprintf(t_name, sizeof(t_name), "Team %c", 'A' + i);
            snprintf(t_tag, sizeof(t_tag), "T%c", 'A' + i);
            snprintf(t_capt, sizeof(t_capt), "Captain %c", 'A' + i);
            snprintf(t_email, sizeof(t_email), "capt.%c@campus.edu", 'a' + i);

            snprintf(teams[i].id, sizeof(teams[i].id), "%s", t_id);
            snprintf(teams[i].name, sizeof(teams[i].name), "%s", t_name);
            teams[i].seed = i + 1;

            sqlite3_bind_text(t_stmt, 1, t_id, -1, SQLITE_STATIC);
            sqlite3_bind_text(t_stmt, 2, comp_id, -1, SQLITE_STATIC);
            sqlite3_bind_text(t_stmt, 3, t_name, -1, SQLITE_STATIC);
            sqlite3_bind_text(t_stmt, 4, t_tag, -1, SQLITE_STATIC);
            sqlite3_bind_int(t_stmt, 5, i + 1);
            sqlite3_bind_text(t_stmt, 6, t_capt, -1, SQLITE_STATIC);
            sqlite3_bind_text(t_stmt, 7, t_email, -1, SQLITE_STATIC);
            sqlite3_bind_int(t_stmt, 8, strcmp(category, "esports") == 0 ? 5 : 11);
            sqlite3_bind_int(t_stmt, 9, i + 1);

            sqlite3_step(t_stmt);
            sqlite3_reset(t_stmt);
        }
        sqlite3_finalize(t_stmt);

        // Update competition registered_teams_count and current_round
        const char *upd = "UPDATE competitions SET registered_teams_count = ?, current_round = 1 WHERE id = ?;";
        sqlite3_stmt *u_stmt = NULL;
        sqlite3_prepare_v2(db, upd, -1, &u_stmt, NULL);
        sqlite3_bind_int(u_stmt, 1, total_teams);
        sqlite3_bind_text(u_stmt, 2, comp_id, -1, SQLITE_STATIC);
        sqlite3_step(u_stmt);
        sqlite3_finalize(u_stmt);

        // Generate fixtures using C DSA modules
        DSAMatch matches[64];
        int match_count = 0;

        if (strcmp(format, "single_elimination") == 0) {
            TreeNode *root = tree_create_bracket(comp_id, teams, total_teams, start_date, &match_count);
            if (root) {
                match_count = tree_flatten_matches(root, matches, 64);
                tree_free(root);
            }
        } else {
            match_count = cyclic_generate_fixtures(comp_id, teams, total_teams, start_date, matches, 64);
        }

        // Insert matches into database
        const char *m_sql = "INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, status, scheduled_time, venue, next_match_id, next_match_slot) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?, 'upcoming', ?, ?, ?, ?);";
        sqlite3_stmt *m_stmt = NULL;
        sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL);

        for (int m = match_count - 1; m >= 0; m--) {
            sqlite3_bind_text(m_stmt, 1, matches[m].id, -1, SQLITE_STATIC);
            sqlite3_bind_text(m_stmt, 2, comp_id, -1, SQLITE_STATIC);
            sqlite3_bind_int(m_stmt, 3, matches[m].round);
            sqlite3_bind_text(m_stmt, 4, matches[m].round_name, -1, SQLITE_STATIC);
            sqlite3_bind_int(m_stmt, 5, matches[m].match_number);

            if (strlen(matches[m].team1_id) > 0) {
                sqlite3_bind_text(m_stmt, 6, matches[m].team1_id, -1, SQLITE_STATIC);
            } else {
                sqlite3_bind_null(m_stmt, 6);
            }

            if (strlen(matches[m].team2_id) > 0) {
                sqlite3_bind_text(m_stmt, 7, matches[m].team2_id, -1, SQLITE_STATIC);
            } else {
                sqlite3_bind_null(m_stmt, 7);
            }

            sqlite3_bind_text(m_stmt, 8, matches[m].scheduled_time, -1, SQLITE_STATIC);
            sqlite3_bind_text(m_stmt, 9, matches[m].venue, -1, SQLITE_STATIC);

            if (strlen(matches[m].next_match_id) > 0) {
                sqlite3_bind_text(m_stmt, 10, matches[m].next_match_id, -1, SQLITE_STATIC);
                sqlite3_bind_int(m_stmt, 11, matches[m].next_match_slot);
            } else {
                sqlite3_bind_null(m_stmt, 10);
                sqlite3_bind_null(m_stmt, 11);
            }

            sqlite3_step(m_stmt);
            sqlite3_reset(m_stmt);
        }
        sqlite3_finalize(m_stmt);

        // Record history event
        char hist_id[64];
        snprintf(hist_id, sizeof(hist_id), "hist-gen-%ld", (long)time(NULL));
        char hist_desc[128];
        snprintf(hist_desc, sizeof(hist_desc), "Generated %d fixtures (%s)", match_count,
            strcmp(format, "round_robin") == 0 ? "Round Robin" : "Single Elimination");

        const char *h_sql = "INSERT INTO history_records (id, competition_id, action_type, description, timestamp, can_undo) VALUES (?, ?, 'FIXTURES_GENERATED', ?, 'Just now', 0);";
        sqlite3_stmt *h_stmt = NULL;
        sqlite3_prepare_v2(db, h_sql, -1, &h_stmt, NULL);
        sqlite3_bind_text(h_stmt, 1, hist_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(h_stmt, 2, comp_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(h_stmt, 3, hist_desc, -1, SQLITE_STATIC);
        sqlite3_step(h_stmt);
        sqlite3_finalize(h_stmt);

        free(teams);
    }

    db_commit_transaction();

    // Query back the newly created competition
    HttpRequest get_req;
    memset(&get_req, 0, sizeof(get_req));
    snprintf(get_req.params[0].key, sizeof(get_req.params[0].key), "id");
    snprintf(get_req.params[0].value, sizeof(get_req.params[0].value), "%s", comp_id);
    get_req.param_count = 1;

    handle_get_competition_by_id(&get_req, client_sock);
}

void handle_update_competition(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    const char *id = http_get_path_param(req, "id");
    if (!db || !id || !req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing ID or payload");
        return;
    }

    char name[128] = {0};
    char status[32] = {0};
    char desc[256] = {0};

    json_get_string(req->body, "name", name, sizeof(name));
    json_get_string(req->body, "status", status, sizeof(status));
    json_get_string(req->body, "description", desc, sizeof(desc));

    char sql[512] = "UPDATE competitions SET ";
    bool has_field = false;

    if (name[0]) {
        strcat(sql, "name = @name");
        has_field = true;
    }
    if (status[0]) {
        if (has_field) strcat(sql, ", ");
        strcat(sql, "status = @status");
        has_field = true;
    }
    if (desc[0]) {
        if (has_field) strcat(sql, ", ");
        strcat(sql, "description = @desc");
        has_field = true;
    }

    if (!has_field) {
        response_send_error(client_sock, 400, "VALIDATION_ERROR", "No update fields provided");
        return;
    }

    strcat(sql, " WHERE id = @id;");

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    int id_idx = sqlite3_bind_parameter_index(stmt, "@id");
    sqlite3_bind_text(stmt, id_idx, id, -1, SQLITE_STATIC);

    if (name[0]) {
        int idx = sqlite3_bind_parameter_index(stmt, "@name");
        sqlite3_bind_text(stmt, idx, name, -1, SQLITE_STATIC);
    }
    if (status[0]) {
        int idx = sqlite3_bind_parameter_index(stmt, "@status");
        sqlite3_bind_text(stmt, idx, status, -1, SQLITE_STATIC);
    }
    if (desc[0]) {
        int idx = sqlite3_bind_parameter_index(stmt, "@desc");
        sqlite3_bind_text(stmt, idx, desc, -1, SQLITE_STATIC);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        handle_get_competition_by_id(req, client_sock);
    } else {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
    }
}
