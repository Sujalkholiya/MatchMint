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
#include <ctype.h>

/* Helper to convert date/time string to unix epoch */
static int64_t parse_time_to_epoch(const char *time_str) {
    if (!time_str || strlen(time_str) == 0) return 0;

    int year = 0, month = 0, day = 0, hour = 0, minute = 0;
    char ampm[8] = {0};

    // Format: YYYY-MM-DD HH:MM AM/PM
    if (sscanf(time_str, "%d-%d-%d %d:%d %2s", &year, &month, &day, &hour, &minute, ampm) == 6) {
        if ((ampm[0] == 'P' || ampm[0] == 'p') && hour < 12) hour += 12;
        if ((ampm[0] == 'A' || ampm[0] == 'a') && hour == 12) hour = 0;
    } else {
        // Format: YYYY-MM-DD HH:MM
        sscanf(time_str, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute);
    }

    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31) return 0;

    struct tm tm_val;
    memset(&tm_val, 0, sizeof(struct tm));
    tm_val.tm_year = year - 1900;
    tm_val.tm_mon = month - 1;
    tm_val.tm_mday = day;
    tm_val.tm_hour = hour;
    tm_val.tm_min = minute;
    tm_val.tm_isdst = -1;

    time_t epoch = mktime(&tm_val);
    return (int64_t)(epoch > 0 ? epoch : 0);
}

/* Helper to serialize a match row to JSON */
static void serialize_match_row(sqlite3_stmt *stmt, JsonBuffer *jb) {
    const char *id = (const char *)sqlite3_column_text(stmt, 0);
    const char *comp_id = (const char *)sqlite3_column_text(stmt, 1);
    int round = sqlite3_column_int(stmt, 2);
    const char *round_name = (const char *)sqlite3_column_text(stmt, 3);
    int match_num = sqlite3_column_int(stmt, 4);
    const char *t1_id = (const char *)sqlite3_column_text(stmt, 5);
    const char *t2_id = (const char *)sqlite3_column_text(stmt, 6);
    int t1_score_type = sqlite3_column_type(stmt, 7);
    int t2_score_type = sqlite3_column_type(stmt, 8);
    const char *winner_id = (const char *)sqlite3_column_text(stmt, 9);
    const char *status = (const char *)sqlite3_column_text(stmt, 10);
    const char *sched_time = (const char *)sqlite3_column_text(stmt, 11);
    const char *venue = (const char *)sqlite3_column_text(stmt, 12);
    const char *next_m_id = (const char *)sqlite3_column_text(stmt, 13);
    int next_slot_type = sqlite3_column_type(stmt, 14);
    const char *tie_details = (const char *)sqlite3_column_text(stmt, 15);

    // Additional team details from JOIN
    const char *t1_name = (const char *)sqlite3_column_text(stmt, 16);
    const char *t1_tag = (const char *)sqlite3_column_text(stmt, 17);
    int t1_seed = sqlite3_column_int(stmt, 18);

    const char *t2_name = (const char *)sqlite3_column_text(stmt, 19);
    const char *t2_tag = (const char *)sqlite3_column_text(stmt, 20);
    int t2_seed = sqlite3_column_int(stmt, 21);

    char esc_round_name[128];
    json_escape_string(round_name ? round_name : "", esc_round_name, sizeof(esc_round_name));

    char esc_venue[128];
    json_escape_string(venue ? venue : "", esc_venue, sizeof(esc_venue));

    json_buffer_append_printf(
        jb,
        "{\"id\":\"%s\",\"competitionId\":\"%s\",\"round\":%d,\"roundName\":\"%s\",\"matchNumber\":%d,",
        id ? id : "",
        comp_id ? comp_id : "",
        round,
        esc_round_name,
        match_num
    );

    // team1Id & team2Id
    if (t1_id) json_buffer_append_printf(jb, "\"team1Id\":\"%s\",", t1_id);
    else json_buffer_append(jb, "\"team1Id\":null,");

    if (t2_id) json_buffer_append_printf(jb, "\"team2Id\":\"%s\",", t2_id);
    else json_buffer_append(jb, "\"team2Id\":null,");

    // team1Score & team2Score
    if (t1_score_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team1Score\":%d,", sqlite3_column_int(stmt, 7));
    else json_buffer_append(jb, "\"team1Score\":null,");

    if (t2_score_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team2Score\":%d,", sqlite3_column_int(stmt, 8));
    else json_buffer_append(jb, "\"team2Score\":null,");

    // winnerId
    if (winner_id) json_buffer_append_printf(jb, "\"winnerId\":\"%s\",", winner_id);
    else json_buffer_append(jb, "\"winnerId\":null,");

    // status, scheduledTime, venue
    json_buffer_append_printf(
        jb,
        "\"status\":\"%s\",\"scheduledTime\":\"%s\",\"venue\":\"%s\",",
        status ? status : "upcoming",
        sched_time ? sched_time : "",
        esc_venue
    );

    // nextMatchId & nextMatchSlot
    if (next_m_id) json_buffer_append_printf(jb, "\"nextMatchId\":\"%s\",", next_m_id);
    else json_buffer_append(jb, "\"nextMatchId\":null,");

    if (next_slot_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"nextMatchSlot\":%d,", sqlite3_column_int(stmt, 14));
    else json_buffer_append(jb, "\"nextMatchSlot\":null,");

    if (tie_details) json_buffer_append_printf(jb, "\"tieBreakerDetails\":\"%s\",", tie_details);
    else json_buffer_append(jb, "\"tieBreakerDetails\":null,");

    // team1 summary
    if (t1_id && t1_name) {
        char esc_t1[128];
        json_escape_string(t1_name, esc_t1, sizeof(esc_t1));
        json_buffer_append_printf(jb, "\"team1\":{\"id\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d},", t1_id, esc_t1, t1_tag ? t1_tag : "", t1_seed);
    } else {
        json_buffer_append(jb, "\"team1\":null,");
    }

    // team2 summary
    if (t2_id && t2_name) {
        char esc_t2[128];
        json_escape_string(t2_name, esc_t2, sizeof(esc_t2));
        json_buffer_append_printf(jb, "\"team2\":{\"id\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d}", t2_id, esc_t2, t2_tag ? t2_tag : "", t2_seed);
    } else {
        json_buffer_append(jb, "\"team2\":null");
    }

    json_buffer_append(jb, "}");
}

/* GET /api/v1/matches */
void handle_get_matches(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database handle unavailable");
        return;
    }

    char comp_filter[64] = {0};
    char status_filter[32] = {0};
    char round_str[16] = {0};
    int round_filter = -1;

    http_get_query_param(req->query, "competitionId", comp_filter, sizeof(comp_filter));
    http_get_query_param(req->query, "status", status_filter, sizeof(status_filter));
    if (http_get_query_param(req->query, "round", round_str, sizeof(round_str))) {
        round_filter = atoi(round_str);
    }

    char sql[512] = "SELECT m.id, m.competition_id, m.round, m.round_name, m.match_number, "
                    "m.team1_id, m.team2_id, m.team1_score, m.team2_score, m.winner_id, "
                    "m.status, m.scheduled_time, m.venue, m.next_match_id, m.next_match_slot, "
                    "m.tie_breaker_details, "
                    "t1.name, t1.short_tag, t1.seed, "
                    "t2.name, t2.short_tag, t2.seed "
                    "FROM matches m "
                    "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                    "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                    "WHERE 1=1 ";

    if (comp_filter[0]) strcat(sql, "AND m.competition_id = ? ");
    if (status_filter[0]) strcat(sql, "AND m.status = ? ");
    if (round_filter >= 0) strcat(sql, "AND m.round = ? ");
    strcat(sql, "ORDER BY m.round ASC, m.match_number ASC;");

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    int bind_idx = 1;
    if (comp_filter[0]) sqlite3_bind_text(stmt, bind_idx++, comp_filter, -1, SQLITE_STATIC);
    if (status_filter[0]) sqlite3_bind_text(stmt, bind_idx++, status_filter, -1, SQLITE_STATIC);
    if (round_filter >= 0) sqlite3_bind_int(stmt, bind_idx++, round_filter);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;
        serialize_match_row(stmt, jb);
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Matches retrieved", jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/matches/:id */
void handle_get_match_by_id(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database handle unavailable");
        return;
    }

    const char *id = req->params[0].value;
    const char *sql = "SELECT m.id, m.competition_id, m.round, m.round_name, m.match_number, "
                      "m.team1_id, m.team2_id, m.team1_score, m.team2_score, m.winner_id, "
                      "m.status, m.scheduled_time, m.venue, m.next_match_id, m.next_match_slot, "
                      "m.tie_breaker_details, "
                      "t1.name, t1.short_tag, t1.seed, "
                      "t2.name, t2.short_tag, t2.seed "
                      "FROM matches m "
                      "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                      "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                      "WHERE m.id = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        JsonBuffer *jb = json_buffer_create();
        serialize_match_row(stmt, jb);
        sqlite3_finalize(stmt);

        response_send_success(client_sock, 200, "Match details retrieved", jb->data);
        json_buffer_free(jb);
    } else {
        sqlite3_finalize(stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Match not found");
    }
}

/* POST /api/v1/competitions/:id/generate-fixtures */
void handle_generate_fixtures(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database handle unavailable");
        return;
    }

    const char *comp_id = req->params[0].value;

    // 1. Fetch competition information
    const char *c_sql = "SELECT name, format, start_date, match_duration_minutes, total_teams FROM competitions WHERE id = ?;";
    sqlite3_stmt *c_stmt = NULL;
    if (sqlite3_prepare_v2(db, c_sql, -1, &c_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(c_stmt, 1, comp_id, -1, SQLITE_STATIC);
    if (sqlite3_step(c_stmt) != SQLITE_ROW) {
        sqlite3_finalize(c_stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Competition not found");
        return;
    }

    char comp_name[128] = {0};
    char format[32] = {0};
    char start_date[32] = {0};
    int duration = sqlite3_column_int(c_stmt, 3);
    if (duration <= 0) duration = 60;

    snprintf(comp_name, sizeof(comp_name), "%s", (const char *)sqlite3_column_text(c_stmt, 0));
    snprintf(format, sizeof(format), "%s", (const char *)sqlite3_column_text(c_stmt, 1));
    const char *s_date = (const char *)sqlite3_column_text(c_stmt, 2);
    snprintf(start_date, sizeof(start_date), "%s", s_date ? s_date : "2026-10-30");
    sqlite3_finalize(c_stmt);

    // 2. Fetch participating teams ordered by seed
    const char *t_sql = "SELECT id, name, short_tag, seed, captain, roster_count, status FROM teams WHERE competition_id = ? ORDER BY seed ASC, name ASC;";
    sqlite3_stmt *t_stmt = NULL;
    if (sqlite3_prepare_v2(db, t_sql, -1, &t_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

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

    if (team_count < 2) {
        response_send_error(client_sock, 400, "INSUFFICIENT_TEAMS", "At least 2 teams are required to generate fixtures.");
        return;
    }

    // 3. Generate matches using C DSA algorithms
    DSAMatch matches[128];
    int match_count = 0;
    int total_rounds = 1;

    if (strcmp(format, "single_elimination") == 0) {
        TreeNode *root = tree_create_bracket(comp_id, teams, team_count, start_date, &match_count);
        if (root) {
            match_count = tree_flatten_matches(root, matches, 128);
            total_rounds = root->match.round;
            tree_free(root);
        }
    } else {
        match_count = cyclic_generate_fixtures(comp_id, teams, team_count, start_date, matches, 128);
        total_rounds = team_count % 2 == 0 ? team_count - 1 : team_count;
    }

    // 4. Begin atomic SQLite transaction to replace fixtures and update status
    db_begin_transaction();

    // Remove existing matches for this competition
    char del_sql[128];
    snprintf(del_sql, sizeof(del_sql), "DELETE FROM matches WHERE competition_id = '%s';", comp_id);
    sqlite3_exec(db, del_sql, NULL, NULL, NULL);

    // Insert new fixtures
    const char *ins_m = "INSERT INTO matches (id, competition_id, round, round_name, match_number, team1_id, team2_id, status, scheduled_time, venue, next_match_id, next_match_slot) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, 'upcoming', ?, ?, ?, ?);";
    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, ins_m, -1, &m_stmt, NULL) != SQLITE_OK) {
        db_rollback_transaction();
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    for (int i = match_count - 1; i >= 0; i--) {
        sqlite3_bind_text(m_stmt, 1, matches[i].id, -1, SQLITE_STATIC);
        sqlite3_bind_text(m_stmt, 2, comp_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(m_stmt, 3, matches[i].round);
        sqlite3_bind_text(m_stmt, 4, matches[i].round_name, -1, SQLITE_STATIC);
        sqlite3_bind_int(m_stmt, 5, matches[i].match_number);

        if (strlen(matches[i].team1_id) > 0) sqlite3_bind_text(m_stmt, 6, matches[i].team1_id, -1, SQLITE_STATIC);
        else sqlite3_bind_null(m_stmt, 6);

        if (strlen(matches[i].team2_id) > 0) sqlite3_bind_text(m_stmt, 7, matches[i].team2_id, -1, SQLITE_STATIC);
        else sqlite3_bind_null(m_stmt, 7);

        sqlite3_bind_text(m_stmt, 8, matches[i].scheduled_time, -1, SQLITE_STATIC);
        sqlite3_bind_text(m_stmt, 9, matches[i].venue, -1, SQLITE_STATIC);

        if (strlen(matches[i].next_match_id) > 0) {
            sqlite3_bind_text(m_stmt, 10, matches[i].next_match_id, -1, SQLITE_STATIC);
            sqlite3_bind_int(m_stmt, 11, matches[i].next_match_slot);
        } else {
            sqlite3_bind_null(m_stmt, 10);
            sqlite3_bind_null(m_stmt, 11);
        }

        sqlite3_step(m_stmt);
        sqlite3_reset(m_stmt);
    }
    sqlite3_finalize(m_stmt);

    // Update competition status to live and total_rounds
    const char *upd_c = "UPDATE competitions SET status = 'live', current_round = 1, total_rounds = ? WHERE id = ?;";
    sqlite3_stmt *u_stmt = NULL;
    sqlite3_prepare_v2(db, upd_c, -1, &u_stmt, NULL);
    sqlite3_bind_int(u_stmt, 1, total_rounds);
    sqlite3_bind_text(u_stmt, 2, comp_id, -1, SQLITE_STATIC);
    sqlite3_step(u_stmt);
    sqlite3_finalize(u_stmt);

    // Record audit history entry
    char hist_id[64];
    snprintf(hist_id, sizeof(hist_id), "hist-gen-%ld", (long)time(NULL));
    char desc[256];
    snprintf(desc, sizeof(desc), "Generated %d fixtures (%s)", match_count, strcmp(format, "round_robin") == 0 ? "Round Robin" : "Single Elimination");

    const char *h_sql = "INSERT INTO history_records (id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo) "
                        "VALUES (?, ?, NULL, 'FIXTURES_GENERATED', ?, datetime('now'), NULL, 0);";
    sqlite3_stmt *h_stmt = NULL;
    sqlite3_prepare_v2(db, h_sql, -1, &h_stmt, NULL);
    sqlite3_bind_text(h_stmt, 1, hist_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 2, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(h_stmt, 3, desc, -1, SQLITE_STATIC);
    sqlite3_step(h_stmt);
    sqlite3_finalize(h_stmt);

    db_commit_transaction();

    // Prepare response
    JsonBuffer *jb = json_buffer_create();
    json_buffer_append_printf(
        jb,
        "{\"competitionId\":\"%s\",\"format\":\"%s\",\"fixturesCount\":%d,\"totalRounds\":%d}",
        comp_id,
        format,
        match_count,
        total_rounds
    );

    char msg[256];
    snprintf(msg, sizeof(msg), "Generated %d fixtures for %s.", match_count, comp_name);
    response_send_success(client_sock, 201, msg, jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/competitions/:id/bracket */
void handle_get_bracket(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database handle unavailable");
        return;
    }

    const char *comp_id = req->params[0].value;

    // Check competition exists
    const char *c_sql = "SELECT format, total_rounds FROM competitions WHERE id = ?;";
    sqlite3_stmt *c_stmt = NULL;
    if (sqlite3_prepare_v2(db, c_sql, -1, &c_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(c_stmt, 1, comp_id, -1, SQLITE_STATIC);
    if (sqlite3_step(c_stmt) != SQLITE_ROW) {
        sqlite3_finalize(c_stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Competition not found");
        return;
    }

    char format[32] = {0};
    int total_rounds = sqlite3_column_int(c_stmt, 1);
    snprintf(format, sizeof(format), "%s", (const char *)sqlite3_column_text(c_stmt, 0));
    sqlite3_finalize(c_stmt);

    // Fetch all matches for competition
    const char *m_sql = "SELECT m.id, m.competition_id, m.round, m.round_name, m.match_number, "
                        "m.team1_id, m.team2_id, m.team1_score, m.team2_score, m.winner_id, "
                        "m.status, m.scheduled_time, m.venue, m.next_match_id, m.next_match_slot, "
                        "m.tie_breaker_details, "
                        "t1.name, t1.short_tag, t1.seed, "
                        "t2.name, t2.short_tag, t2.seed "
                        "FROM matches m "
                        "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                        "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                        "WHERE m.competition_id = ? "
                        "ORDER BY m.round ASC, m.match_number ASC;";

    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(m_stmt, 1, comp_id, -1, SQLITE_STATIC);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append_printf(
        jb,
        "{\"competitionId\":\"%s\",\"format\":\"%s\",\"totalRounds\":%d,\"rounds\":[",
        comp_id,
        format,
        total_rounds
    );

    int current_round = -1;
    bool round_first = true;
    bool match_first = true;

    while (sqlite3_step(m_stmt) == SQLITE_ROW) {
        int r = sqlite3_column_int(m_stmt, 2);
        const char *r_name = (const char *)sqlite3_column_text(m_stmt, 3);

        if (r != current_round) {
            if (current_round != -1) {
                // Close previous matches array and round object
                json_buffer_append(jb, "]}");
            }

            if (!round_first) json_buffer_append(jb, ",");
            round_first = false;

            current_round = r;
            match_first = true;

            char esc_r_name[128];
            json_escape_string(r_name ? r_name : "", esc_r_name, sizeof(esc_r_name));
            json_buffer_append_printf(jb, "{\"round\":%d,\"roundName\":\"%s\",\"matches\":[", r, esc_r_name);
        }

        if (!match_first) json_buffer_append(jb, ",");
        match_first = false;

        serialize_match_row(m_stmt, jb);
    }

    if (current_round != -1) {
        json_buffer_append(jb, "]}");
    }

    json_buffer_append(jb, "]}");
    sqlite3_finalize(m_stmt);

    response_send_success(client_sock, 200, "Bracket visualization structure", jb->data);
    json_buffer_free(jb);
}

/* PATCH /api/v1/matches/:id/schedule */
void handle_reschedule_match(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db || !req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing request body");
        return;
    }

    const char *match_id = req->params[0].value;
    char scheduled_time[64] = {0};
    char venue[64] = {0};

    json_get_string(req->body, "scheduledTime", scheduled_time, sizeof(scheduled_time));
    json_get_string(req->body, "venue", venue, sizeof(venue));

    if (strlen(scheduled_time) == 0 || strlen(venue) == 0) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Both 'scheduledTime' and 'venue' are required");
        return;
    }

    // 1. Fetch current match details and competition duration
    const char *m_sql = "SELECT m.competition_id, c.match_duration_minutes, m.round_name, m.match_number "
                        "FROM matches m "
                        "JOIN competitions c ON m.competition_id = c.id "
                        "WHERE m.id = ?;";
    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(m_stmt, 1, match_id, -1, SQLITE_STATIC);
    if (sqlite3_step(m_stmt) != SQLITE_ROW) {
        sqlite3_finalize(m_stmt);
        response_send_error(client_sock, 404, "NOT_FOUND", "Match not found");
        return;
    }

    char comp_id[64] = {0};
    snprintf(comp_id, sizeof(comp_id), "%s", (const char *)sqlite3_column_text(m_stmt, 0));
    int duration = sqlite3_column_int(m_stmt, 1);
    if (duration <= 0) duration = 60;
    int target_match_number = sqlite3_column_int(m_stmt, 3);
    sqlite3_finalize(m_stmt);

    // 2. Interval conflict detection using pure C DSA interval module
    int64_t target_start = parse_time_to_epoch(scheduled_time);
    int64_t target_end = target_start + (duration * 60);

    // Query other matches scheduled in the same competition or venue
    const char *q_sql = "SELECT id, scheduled_time, venue, match_number FROM matches WHERE competition_id = ? AND id != ?;";
    sqlite3_stmt *q_stmt = NULL;
    sqlite3_prepare_v2(db, q_sql, -1, &q_stmt, NULL);
    sqlite3_bind_text(q_stmt, 1, comp_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(q_stmt, 2, match_id, -1, SQLITE_STATIC);

    DSAConflict detected_conflicts[16];
    int conflict_count = 0;

    while (sqlite3_step(q_stmt) == SQLITE_ROW && conflict_count < 16) {
        const char *other_id = (const char *)sqlite3_column_text(q_stmt, 0);
        const char *other_time = (const char *)sqlite3_column_text(q_stmt, 1);
        const char *other_venue = (const char *)sqlite3_column_text(q_stmt, 2);
        int other_match_number = sqlite3_column_int(q_stmt, 3);

        if (other_venue && strcmp(other_venue, venue) == 0 && other_time) {
            int64_t other_start = parse_time_to_epoch(other_time);
            int64_t other_end = other_start + (duration * 60);

            if (target_start > 0 && other_start > 0 && intervals_overlap(target_start, target_end, other_start, other_end)) {
                DSAConflict *c = &detected_conflicts[conflict_count++];
                snprintf(c->type, sizeof(c->type), "VENUE_COLLISION");
                snprintf(c->match1_id, sizeof(c->match1_id), "%s", match_id);
                snprintf(c->match2_id, sizeof(c->match2_id), "%s", other_id);
                snprintf(c->venue, sizeof(c->venue), "%s", venue);
                snprintf(c->time_slot, sizeof(c->time_slot), "%s & %s", scheduled_time, other_time);
                snprintf(c->description, sizeof(c->description), "Venue clash at \"%s\": Match #%d and Match #%d overlap.", venue, target_match_number, other_match_number);
            }
        }
    }
    sqlite3_finalize(q_stmt);

    // 3. Update match record in SQLite
    const char *upd_sql = "UPDATE matches SET scheduled_time = ?, venue = ? WHERE id = ?;";
    sqlite3_stmt *u_stmt = NULL;
    sqlite3_prepare_v2(db, upd_sql, -1, &u_stmt, NULL);
    sqlite3_bind_text(u_stmt, 1, scheduled_time, -1, SQLITE_STATIC);
    sqlite3_bind_text(u_stmt, 2, venue, -1, SQLITE_STATIC);
    sqlite3_bind_text(u_stmt, 3, match_id, -1, SQLITE_STATIC);
    sqlite3_step(u_stmt);
    sqlite3_finalize(u_stmt);

    // 4. Build response JSON including match object and conflicts array
    const char *fetch_sql = "SELECT m.id, m.competition_id, m.round, m.round_name, m.match_number, "
                           "m.team1_id, m.team2_id, m.team1_score, m.team2_score, m.winner_id, "
                           "m.status, m.scheduled_time, m.venue, m.next_match_id, m.next_match_slot, "
                           "m.tie_breaker_details, "
                           "t1.name, t1.short_tag, t1.seed, "
                           "t2.name, t2.short_tag, t2.seed "
                           "FROM matches m "
                           "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                           "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                           "WHERE m.id = ?;";

    sqlite3_stmt *f_stmt = NULL;
    sqlite3_prepare_v2(db, fetch_sql, -1, &f_stmt, NULL);
    sqlite3_bind_text(f_stmt, 1, match_id, -1, SQLITE_STATIC);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "{\"match\":");

    if (sqlite3_step(f_stmt) == SQLITE_ROW) {
        serialize_match_row(f_stmt, jb);
    } else {
        json_buffer_append(jb, "null");
    }
    sqlite3_finalize(f_stmt);

    json_buffer_append(jb, ",\"conflicts\":[");

    for (int i = 0; i < conflict_count; i++) {
        if (i > 0) json_buffer_append(jb, ",");
        char esc_desc[256];
        json_escape_string(detected_conflicts[i].description, esc_desc, sizeof(esc_desc));
        json_buffer_append_printf(
            jb,
            "{\"type\":\"%s\",\"match1Id\":\"%s\",\"match2Id\":\"%s\",\"venue\":\"%s\",\"timeSlot\":\"%s\",\"description\":\"%s\"}",
            detected_conflicts[i].type,
            detected_conflicts[i].match1_id,
            detected_conflicts[i].match2_id,
            detected_conflicts[i].venue,
            detected_conflicts[i].time_slot,
            esc_desc
        );
    }
    json_buffer_append(jb, "]}");

    response_send_success(client_sock, 200, "Match schedule updated.", jb->data);
    json_buffer_free(jb);
}
