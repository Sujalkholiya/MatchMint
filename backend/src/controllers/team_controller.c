#include "controllers.h"
#include "database.h"
#include "response.h"
#include "json_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static void serialize_team_row(sqlite3_stmt *stmt, JsonBuffer *jb) {
    const char *id = (const char *)sqlite3_column_text(stmt, 0);
    const char *comp_id = (const char *)sqlite3_column_text(stmt, 1);
    const char *name = (const char *)sqlite3_column_text(stmt, 2);
    const char *short_tag = (const char *)sqlite3_column_text(stmt, 3);
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
        comp_id ? comp_id : "",
        esc_name,
        short_tag ? short_tag : "",
        seed,
        esc_captain,
        email ? email : "",
        roster,
        wins,
        draws,
        losses,
        points,
        score_for,
        score_against,
        score_diff,
        rank,
        status ? status : "active"
    );
}

void handle_get_teams(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    char comp_filter[64] = {0};
    http_get_query_param(req->query, "competitionId", comp_filter, sizeof(comp_filter));

    char sql[512] = "SELECT id, competition_id, name, short_tag, seed, captain, email, "
                    "roster_count, wins, draws, losses, points, score_for, score_against, "
                    "score_diff, rank, status FROM teams";

    if (comp_filter[0]) {
        strcat(sql, " WHERE competition_id = ?");
    }
    strcat(sql, " ORDER BY rank ASC, seed ASC;");

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "QUERY_ERROR", sqlite3_errmsg(db));
        return;
    }

    if (comp_filter[0]) {
        sqlite3_bind_text(stmt, 1, comp_filter, -1, SQLITE_STATIC);
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;
        serialize_team_row(stmt, jb);
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Teams retrieved successfully", jb->data);
    json_buffer_free(jb);
}

void handle_get_team_by_id(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    const char *id = http_get_path_param(req, "id");
    if (!db || !id) {
        response_send_error(client_sock, 400, "INVALID_REQUEST", "Missing team ID");
        return;
    }

    const char *sql = "SELECT id, competition_id, name, short_tag, seed, captain, email, "
                      "roster_count, wins, draws, losses, points, score_for, score_against, "
                      "score_diff, rank, status FROM teams WHERE id = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "QUERY_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        JsonBuffer *jb = json_buffer_create();
        serialize_team_row(stmt, jb);
        sqlite3_finalize(stmt);

        response_send_success(client_sock, 200, "Team details retrieved", jb->data);
        json_buffer_free(jb);
    } else {
        sqlite3_finalize(stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Team not found");
    }
}

static int g_team_seq = 100;

void handle_register_team(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db || !req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing request body");
        return;
    }

    char comp_id[64] = {0};
    char name[128] = {0};
    char captain[128] = {0};
    int roster_count = 5;
    char email[128] = {0};

    json_get_string(req->body, "competitionId", comp_id, sizeof(comp_id));
    json_get_string(req->body, "name", name, sizeof(name));
    json_get_string(req->body, "captain", captain, sizeof(captain));
    json_get_int(req->body, "rosterCount", &roster_count);
    json_get_string(req->body, "email", email, sizeof(email));

    if (strlen(comp_id) == 0 || strlen(name) == 0 || strlen(captain) == 0) {
        response_send_error(client_sock, 400, "VALIDATION_ERROR", "Competition ID, team name, and captain name are required");
        return;
    }

    // Generate short tag (first 3 alphanumeric uppercase characters)
    char tag[8] = {0};
    int t_idx = 0;
    for (size_t i = 0; name[i] != '\0' && t_idx < 3; i++) {
        if (isalnum((unsigned char)name[i])) {
            tag[t_idx++] = (char)toupper((unsigned char)name[i]);
        }
    }
    tag[t_idx] = '\0';
    if (strlen(tag) == 0) strcpy(tag, "SQD");

    // Generate email if missing
    if (strlen(email) == 0) {
        char tag_lower[8];
        for (int i = 0; tag[i] != '\0'; i++) tag_lower[i] = (char)tolower((unsigned char)tag[i]);
        tag_lower[strlen(tag)] = '\0';
        snprintf(email, sizeof(email), "%s.lead@campus.edu", tag_lower);
    }

    db_begin_transaction();

    // Count existing teams for seed & rank
    const char *cnt_sql = "SELECT count(*) FROM teams WHERE competition_id = ?;";
    sqlite3_stmt *c_stmt = NULL;
    sqlite3_prepare_v2(db, cnt_sql, -1, &c_stmt, NULL);
    sqlite3_bind_text(c_stmt, 1, comp_id, -1, SQLITE_STATIC);
    sqlite3_step(c_stmt);
    int current_count = sqlite3_column_int(c_stmt, 0);
    sqlite3_finalize(c_stmt);

    int new_seed = current_count + 1;
    int new_rank = current_count + 1;

    char team_id[64];
    snprintf(team_id, sizeof(team_id), "team-%lld-%d", (long long)time(NULL), ++g_team_seq);

    const char *ins_sql = "INSERT INTO teams (id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, 0, 0, 0, 0, 0, 0, 0, ?, 'active');";
    sqlite3_stmt *i_stmt = NULL;
    sqlite3_prepare_v2(db, ins_sql, -1, &i_stmt, NULL);
    sqlite3_bind_text(i_stmt, 1, team_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(i_stmt, 2, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(i_stmt, 3, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(i_stmt, 4, tag, -1, SQLITE_STATIC);
    sqlite3_bind_int(i_stmt, 5, new_seed);
    sqlite3_bind_text(i_stmt, 6, captain, -1, SQLITE_STATIC);
    sqlite3_bind_text(i_stmt, 7, email, -1, SQLITE_STATIC);
    sqlite3_bind_int(i_stmt, 8, roster_count > 0 ? roster_count : 5);
    sqlite3_bind_int(i_stmt, 9, new_rank);

    int rc = sqlite3_step(i_stmt);
    sqlite3_finalize(i_stmt);

    if (rc != SQLITE_DONE) {
        db_rollback_transaction();
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    // Increment registered_teams_count in competition
    const char *upd_sql = "UPDATE competitions SET registered_teams_count = registered_teams_count + 1 WHERE id = ?;";
    sqlite3_stmt *u_stmt = NULL;
    sqlite3_prepare_v2(db, upd_sql, -1, &u_stmt, NULL);
    sqlite3_bind_text(u_stmt, 1, comp_id, -1, SQLITE_STATIC);
    sqlite3_step(u_stmt);
    sqlite3_finalize(u_stmt);

    db_commit_transaction();

    // Query back newly registered team
    HttpRequest get_req;
    memset(&get_req, 0, sizeof(get_req));
    snprintf(get_req.params[0].key, sizeof(get_req.params[0].key), "id");
    snprintf(get_req.params[0].value, sizeof(get_req.params[0].value), "%s", team_id);
    get_req.param_count = 1;

    handle_get_team_by_id(&get_req, client_sock);
}
