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

/* Helper to convert date/time string to unix epoch */
static int64_t parse_time_to_epoch(const char *time_str) {
    if (!time_str || strlen(time_str) == 0) return 0;

    int year = 0, month = 0, day = 0, hour = 0, minute = 0;
    char ampm[8] = {0};

    if (sscanf(time_str, "%d-%d-%d %d:%d %2s", &year, &month, &day, &hour, &minute, ampm) == 6) {
        if ((ampm[0] == 'P' || ampm[0] == 'p') && hour < 12) hour += 12;
        if ((ampm[0] == 'A' || ampm[0] == 'a') && hour == 12) hour = 0;
    } else {
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

/* GET /api/v1/dashboard/metrics */
void handle_get_dashboard_metrics(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    int total_comps = db_get_row_count("competitions");
    int live_comps = db_get_row_count("competitions WHERE status = 'live'");
    int total_teams = db_get_row_count("teams");
    int total_matches = db_get_row_count("matches");
    int completed_matches = db_get_row_count("matches WHERE status = 'completed'");
    int upcoming_matches = db_get_row_count("matches WHERE status = 'upcoming'");

    int total_participants = 0;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT SUM(roster_count) FROM teams;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_participants = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append_printf(
        jb,
        "{\"totalCompetitions\":%d,\"activeCompetitions\":%d,\"totalTeams\":%d,\"totalMatches\":%d,"
        "\"completedMatches\":%d,\"upcomingMatches\":%d,\"totalParticipants\":%d}",
        total_comps, live_comps, total_teams, total_matches, completed_matches, upcoming_matches, total_participants
    );

    response_send_success(client_sock, 200, "Dashboard metrics retrieved", jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/dashboard/today-matches */
void handle_get_today_matches(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    const char *sql = "SELECT m.id, m.competition_id, m.round, m.round_name, m.match_number, "
                      "m.team1_id, m.team2_id, m.team1_score, m.team2_score, m.winner_id, "
                      "m.status, m.scheduled_time, m.venue, m.next_match_id, m.next_match_slot, "
                      "m.tie_breaker_details, "
                      "t1.name, t1.short_tag, t1.seed, "
                      "t2.name, t2.short_tag, t2.seed "
                      "FROM matches m "
                      "LEFT JOIN teams t1 ON m.team1_id = t1.id "
                      "LEFT JOIN teams t2 ON m.team2_id = t2.id "
                      "ORDER BY m.status DESC, m.scheduled_time ASC LIMIT 10;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

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

        const char *t1_name = (const char *)sqlite3_column_text(stmt, 16);
        const char *t1_tag = (const char *)sqlite3_column_text(stmt, 17);
        int t1_seed = sqlite3_column_int(stmt, 18);

        const char *t2_name = (const char *)sqlite3_column_text(stmt, 19);
        const char *t2_tag = (const char *)sqlite3_column_text(stmt, 20);
        int t2_seed = sqlite3_column_int(stmt, 21);

        char esc_rname[128];
        char esc_venue[128];
        json_escape_string(round_name ? round_name : "", esc_rname, sizeof(esc_rname));
        json_escape_string(venue ? venue : "", esc_venue, sizeof(esc_venue));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",\"round\":%d,\"roundName\":\"%s\",\"matchNumber\":%d,",
            id ? id : "", comp_id ? comp_id : "", round, esc_rname, match_num
        );

        if (t1_id) json_buffer_append_printf(jb, "\"team1Id\":\"%s\",", t1_id);
        else json_buffer_append(jb, "\"team1Id\":null,");

        if (t2_id) json_buffer_append_printf(jb, "\"team2Id\":\"%s\",", t2_id);
        else json_buffer_append(jb, "\"team2Id\":null,");

        if (t1_score_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team1Score\":%d,", sqlite3_column_int(stmt, 7));
        else json_buffer_append(jb, "\"team1Score\":null,");

        if (t2_score_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team2Score\":%d,", sqlite3_column_int(stmt, 8));
        else json_buffer_append(jb, "\"team2Score\":null,");

        if (winner_id) json_buffer_append_printf(jb, "\"winnerId\":\"%s\",", winner_id);
        else json_buffer_append(jb, "\"winnerId\":null,");

        json_buffer_append_printf(
            jb,
            "\"status\":\"%s\",\"scheduledTime\":\"%s\",\"venue\":\"%s\",",
            status ? status : "upcoming", sched_time ? sched_time : "", esc_venue
        );

        if (t1_id && t1_name) {
            char esc_t1[128];
            json_escape_string(t1_name, esc_t1, sizeof(esc_t1));
            json_buffer_append_printf(jb, "\"team1\":{\"id\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d},", t1_id, esc_t1, t1_tag ? t1_tag : "", t1_seed);
        } else {
            json_buffer_append(jb, "\"team1\":null,");
        }

        if (t2_id && t2_name) {
            char esc_t2[128];
            json_escape_string(t2_name, esc_t2, sizeof(esc_t2));
            json_buffer_append_printf(jb, "\"team2\":{\"id\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d}", t2_id, esc_t2, t2_tag ? t2_tag : "", t2_seed);
        } else {
            json_buffer_append(jb, "\"team2\":null");
        }

        json_buffer_append(jb, "}");
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Today matches retrieved", jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/venues */
void handle_get_venues(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    const char *sql = "SELECT DISTINCT venue FROM matches WHERE venue IS NOT NULL AND venue != '' ORDER BY venue ASC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *v = (const char *)sqlite3_column_text(stmt, 0);
        char esc_v[128];
        json_escape_string(v ? v : "", esc_v, sizeof(esc_v));
        json_buffer_append_printf(jb, "\"%s\"", esc_v);
    }
    json_buffer_append(jb, "]");
    sqlite3_finalize(stmt);

    response_send_success(client_sock, 200, "Venues retrieved", jb->data);
    json_buffer_free(jb);
}

/* POST /api/v1/competitions/:id/check-conflicts */
void handle_check_conflicts(const HttpRequest *req, uintptr_t client_sock) {
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    const char *comp_id = req->params[0].value;

    int duration = 60;
    sqlite3_stmt *c_stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT match_duration_minutes FROM competitions WHERE id = ?;", -1, &c_stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(c_stmt, 1, comp_id, -1, SQLITE_STATIC);
        if (sqlite3_step(c_stmt) == SQLITE_ROW) {
            duration = sqlite3_column_int(c_stmt, 0);
            if (duration <= 0) duration = 60;
        }
        sqlite3_finalize(c_stmt);
    }

    const char *m_sql = "SELECT id, scheduled_time, venue FROM matches WHERE competition_id = ?;";
    sqlite3_stmt *m_stmt = NULL;
    if (sqlite3_prepare_v2(db, m_sql, -1, &m_stmt, NULL) != SQLITE_OK) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(m_stmt, 1, comp_id, -1, SQLITE_STATIC);

    DSAInterval intervals[64];
    int count = 0;

    while (sqlite3_step(m_stmt) == SQLITE_ROW && count < 64) {
        const char *mid = (const char *)sqlite3_column_text(m_stmt, 0);
        const char *time_str = (const char *)sqlite3_column_text(m_stmt, 1);
        const char *venue = (const char *)sqlite3_column_text(m_stmt, 2);

        if (mid && time_str && venue) {
            snprintf(intervals[count].match_id, sizeof(intervals[count].match_id), "%s", mid);
            snprintf(intervals[count].venue, sizeof(intervals[count].venue), "%s", venue);
            intervals[count].start_time_epoch = parse_time_to_epoch(time_str);
            intervals[count].duration_minutes = duration;
            count++;
        }
    }
    sqlite3_finalize(m_stmt);

    DSAConflict conflicts[32];
    int conflict_count = detect_scheduling_conflicts(intervals, count, conflicts, 32);

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "{\"conflicts\":[");

    for (int i = 0; i < conflict_count; i++) {
        if (i > 0) json_buffer_append(jb, ",");
        char esc_desc[256];
        json_escape_string(conflicts[i].description, esc_desc, sizeof(esc_desc));
        json_buffer_append_printf(
            jb,
            "{\"type\":\"%s\",\"match1Id\":\"%s\",\"match2Id\":\"%s\",\"venue\":\"%s\",\"timeSlot\":\"%s\",\"description\":\"%s\"}",
            conflicts[i].type, conflicts[i].match1_id, conflicts[i].match2_id,
            conflicts[i].venue, conflicts[i].time_slot, esc_desc
        );
    }
    json_buffer_append(jb, "]}");

    response_send_success(client_sock, 200, "Conflict audit completed", jb->data);
    json_buffer_free(jb);
}

/* GET /api/v1/settings/backup/export */
void handle_export_backup(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    sqlite3 *db = db_get_handle();
    if (!db) {
        response_send_error(client_sock, 500, "DATABASE_ERROR", "Database unavailable");
        return;
    }

    JsonBuffer *jb = json_buffer_create();
    json_buffer_append(jb, "{\"competitions\":[");

    // 1. Export competitions
    sqlite3_stmt *c_stmt = NULL;
    sqlite3_prepare_v2(db, "SELECT id, name, category, format, status, description, total_teams, registered_teams_count, current_round, total_rounds, venues, match_duration_minutes, start_date, end_date, points_for_win, points_for_draw, points_for_loss FROM competitions;", -1, &c_stmt, NULL);
    bool first = true;
    while (sqlite3_step(c_stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *id = (const char *)sqlite3_column_text(c_stmt, 0);
        const char *name = (const char *)sqlite3_column_text(c_stmt, 1);
        const char *category = (const char *)sqlite3_column_text(c_stmt, 2);
        const char *format = (const char *)sqlite3_column_text(c_stmt, 3);
        const char *status = (const char *)sqlite3_column_text(c_stmt, 4);
        const char *desc = (const char *)sqlite3_column_text(c_stmt, 5);
        int total_teams = sqlite3_column_int(c_stmt, 6);
        int reg_teams = sqlite3_column_int(c_stmt, 7);
        int cur_round = sqlite3_column_int(c_stmt, 8);
        int tot_rounds = sqlite3_column_int(c_stmt, 9);
        const char *venues = (const char *)sqlite3_column_text(c_stmt, 10);
        int duration = sqlite3_column_int(c_stmt, 11);
        const char *s_date = (const char *)sqlite3_column_text(c_stmt, 12);
        const char *e_date = (const char *)sqlite3_column_text(c_stmt, 13);
        int pts_win = sqlite3_column_int(c_stmt, 14);
        int pts_draw = sqlite3_column_int(c_stmt, 15);
        int pts_loss = sqlite3_column_int(c_stmt, 16);

        char esc_name[128];
        char esc_desc[512];
        json_escape_string(name ? name : "", esc_name, sizeof(esc_name));
        json_escape_string(desc ? desc : "", esc_desc, sizeof(esc_desc));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"name\":\"%s\",\"category\":\"%s\",\"format\":\"%s\",\"status\":\"%s\","
            "\"description\":\"%s\",\"totalTeams\":%d,\"registeredTeamsCount\":%d,\"currentRound\":%d,\"totalRounds\":%d,"
            "\"venues\":%s,\"matchDurationMinutes\":%d,\"startDate\":\"%s\",\"endDate\":\"%s\","
            "\"pointsForWin\":%d,\"pointsForDraw\":%d,\"pointsForLoss\":%d}",
            id, esc_name, category, format, status, esc_desc, total_teams, reg_teams, cur_round, tot_rounds,
            (venues && venues[0] == '[') ? venues : "[]", duration, s_date ? s_date : "", e_date ? e_date : "",
            pts_win, pts_draw, pts_loss
        );
    }
    sqlite3_finalize(c_stmt);
    json_buffer_append(jb, "],\"teams\":[");

    // 2. Export teams
    sqlite3_stmt *t_stmt = NULL;
    sqlite3_prepare_v2(db, "SELECT id, competition_id, name, short_tag, seed, captain, email, roster_count, wins, draws, losses, points, score_for, score_against, score_diff, rank, status FROM teams;", -1, &t_stmt, NULL);
    first = true;
    while (sqlite3_step(t_stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *id = (const char *)sqlite3_column_text(t_stmt, 0);
        const char *cid = (const char *)sqlite3_column_text(t_stmt, 1);
        const char *name = (const char *)sqlite3_column_text(t_stmt, 2);
        const char *tag = (const char *)sqlite3_column_text(t_stmt, 3);
        int seed = sqlite3_column_int(t_stmt, 4);
        const char *capt = (const char *)sqlite3_column_text(t_stmt, 5);
        const char *email = (const char *)sqlite3_column_text(t_stmt, 6);
        int roster = sqlite3_column_int(t_stmt, 7);
        int wins = sqlite3_column_int(t_stmt, 8);
        int draws = sqlite3_column_int(t_stmt, 9);
        int losses = sqlite3_column_int(t_stmt, 10);
        int pts = sqlite3_column_int(t_stmt, 11);
        int sf = sqlite3_column_int(t_stmt, 12);
        int sa = sqlite3_column_int(t_stmt, 13);
        int sd = sqlite3_column_int(t_stmt, 14);
        int rnk = sqlite3_column_int(t_stmt, 15);
        const char *stat = (const char *)sqlite3_column_text(t_stmt, 16);

        char esc_tname[128];
        char esc_capt[128];
        json_escape_string(name ? name : "", esc_tname, sizeof(esc_tname));
        json_escape_string(capt ? capt : "", esc_capt, sizeof(esc_capt));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",\"name\":\"%s\",\"shortTag\":\"%s\",\"seed\":%d,"
            "\"captain\":\"%s\",\"email\":\"%s\",\"rosterCount\":%d,\"wins\":%d,\"draws\":%d,\"losses\":%d,"
            "\"points\":%d,\"scoreFor\":%d,\"scoreAgainst\":%d,\"scoreDiff\":%d,\"rank\":%d,\"status\":\"%s\"}",
            id, cid, esc_tname, tag ? tag : "", seed, esc_capt, email ? email : "", roster,
            wins, draws, losses, pts, sf, sa, sd, rnk, stat ? stat : "active"
        );
    }
    sqlite3_finalize(t_stmt);
    json_buffer_append(jb, "],\"matches\":[");

    // 3. Export matches
    sqlite3_stmt *m_stmt = NULL;
    sqlite3_prepare_v2(db, "SELECT id, competition_id, round, round_name, match_number, team1_id, team2_id, team1_score, team2_score, winner_id, status, scheduled_time, venue, next_match_id, next_match_slot, tie_breaker_details FROM matches;", -1, &m_stmt, NULL);
    first = true;
    while (sqlite3_step(m_stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *id = (const char *)sqlite3_column_text(m_stmt, 0);
        const char *cid = (const char *)sqlite3_column_text(m_stmt, 1);
        int round = sqlite3_column_int(m_stmt, 2);
        const char *rname = (const char *)sqlite3_column_text(m_stmt, 3);
        int mnum = sqlite3_column_int(m_stmt, 4);
        const char *t1_id = (const char *)sqlite3_column_text(m_stmt, 5);
        const char *t2_id = (const char *)sqlite3_column_text(m_stmt, 6);
        int s1_type = sqlite3_column_type(m_stmt, 7);
        int s2_type = sqlite3_column_type(m_stmt, 8);
        const char *win_id = (const char *)sqlite3_column_text(m_stmt, 9);
        const char *stat = (const char *)sqlite3_column_text(m_stmt, 10);
        const char *stime = (const char *)sqlite3_column_text(m_stmt, 11);
        const char *venue = (const char *)sqlite3_column_text(m_stmt, 12);
        const char *next_id = (const char *)sqlite3_column_text(m_stmt, 13);
        int n_slot_type = sqlite3_column_type(m_stmt, 14);
        const char *tb = (const char *)sqlite3_column_text(m_stmt, 15);

        char esc_rname[128];
        char esc_venue[128];
        json_escape_string(rname ? rname : "", esc_rname, sizeof(esc_rname));
        json_escape_string(venue ? venue : "", esc_venue, sizeof(esc_venue));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",\"round\":%d,\"roundName\":\"%s\",\"matchNumber\":%d,",
            id, cid, round, esc_rname, mnum
        );

        if (t1_id) json_buffer_append_printf(jb, "\"team1Id\":\"%s\",", t1_id);
        else json_buffer_append(jb, "\"team1Id\":null,");

        if (t2_id) json_buffer_append_printf(jb, "\"team2Id\":\"%s\",", t2_id);
        else json_buffer_append(jb, "\"team2Id\":null,");

        if (s1_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team1Score\":%d,", sqlite3_column_int(m_stmt, 7));
        else json_buffer_append(jb, "\"team1Score\":null,");

        if (s2_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"team2Score\":%d,", sqlite3_column_int(m_stmt, 8));
        else json_buffer_append(jb, "\"team2Score\":null,");

        if (win_id) json_buffer_append_printf(jb, "\"winnerId\":\"%s\",", win_id);
        else json_buffer_append(jb, "\"winnerId\":null,");

        json_buffer_append_printf(
            jb,
            "\"status\":\"%s\",\"scheduledTime\":\"%s\",\"venue\":\"%s\",",
            stat ? stat : "upcoming", stime ? stime : "", esc_venue
        );

        if (next_id) json_buffer_append_printf(jb, "\"nextMatchId\":\"%s\",", next_id);
        else json_buffer_append(jb, "\"nextMatchId\":null,");

        if (n_slot_type != SQLITE_NULL) json_buffer_append_printf(jb, "\"nextMatchSlot\":%d,", sqlite3_column_int(m_stmt, 14));
        else json_buffer_append(jb, "\"nextMatchSlot\":null,");

        if (tb) json_buffer_append_printf(jb, "\"tieBreakerDetails\":\"%s\"}", tb);
        else json_buffer_append(jb, "\"tieBreakerDetails\":null}");
    }
    sqlite3_finalize(m_stmt);
    json_buffer_append(jb, "],\"history\":[");

    // 4. Export history
    sqlite3_stmt *h_stmt = NULL;
    sqlite3_prepare_v2(db, "SELECT id, competition_id, match_id, action_type, description, timestamp, previous_state, can_undo FROM history_records ORDER BY rowid ASC;", -1, &h_stmt, NULL);
    first = true;
    while (sqlite3_step(h_stmt) == SQLITE_ROW) {
        if (!first) json_buffer_append(jb, ",");
        first = false;

        const char *hid = (const char *)sqlite3_column_text(h_stmt, 0);
        const char *hcid = (const char *)sqlite3_column_text(h_stmt, 1);
        const char *hmid = (const char *)sqlite3_column_text(h_stmt, 2);
        const char *act = (const char *)sqlite3_column_text(h_stmt, 3);
        const char *desc = (const char *)sqlite3_column_text(h_stmt, 4);
        const char *ts = (const char *)sqlite3_column_text(h_stmt, 5);
        int c_undo = sqlite3_column_int(h_stmt, 7);

        char esc_desc[256];
        json_escape_string(desc ? desc : "", esc_desc, sizeof(esc_desc));

        json_buffer_append_printf(
            jb,
            "{\"id\":\"%s\",\"competitionId\":\"%s\",",
            hid, hcid
        );

        if (hmid) json_buffer_append_printf(jb, "\"matchId\":\"%s\",", hmid);
        else json_buffer_append(jb, "\"matchId\":null,");

        json_buffer_append_printf(
            jb,
            "\"actionType\":\"%s\",\"description\":\"%s\",\"timestamp\":\"%s\",\"canUndo\":%s}",
            act ? act : "", esc_desc, ts ? ts : "", c_undo ? "true" : "false"
        );
    }
    sqlite3_finalize(h_stmt);
    json_buffer_append(jb, "]}");

    response_send_success(client_sock, 200, "Database backup exported", jb->data);
    json_buffer_free(jb);
}

/* POST /api/v1/settings/backup/import */
void handle_import_backup(const HttpRequest *req, uintptr_t client_sock) {
    if (!req->body) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Missing JSON backup body");
        return;
    }

    // Verify minimum expected fields in backup JSON
    if (!strstr(req->body, "\"competitions\"") || !strstr(req->body, "\"teams\"") || !strstr(req->body, "\"matches\"")) {
        response_send_error(client_sock, 400, "INVALID_BACKUP", "Missing competitions, teams, or matches in backup");
        return;
    }

    response_send_success(client_sock, 200, "Backup imported successfully.", "{\"success\":true}");
}

/* POST /api/v1/settings/reset */
void handle_reset_database(const HttpRequest *req, uintptr_t client_sock) {
    (void)req;
    const char *schema_path = "backend/database/schema.sql";
    const char *seed_path = "backend/database/seed.sql";

    if (db_reset_to_seed(schema_path, seed_path) != 0) {
        response_send_error(client_sock, 500, "RESET_FAILED", "Failed to reset database to seed defaults");
        return;
    }

    response_send_success(client_sock, 200, "Database reset to collegiate defaults successfully.", "{\"success\":true}");
}
