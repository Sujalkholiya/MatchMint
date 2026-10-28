#ifndef MATCHMINT_CONTROLLERS_H
#define MATCHMINT_CONTROLLERS_H

#include "http.h"
#include <stdint.h>

/* Competition Controllers */
void handle_get_competitions(const HttpRequest *req, uintptr_t client_sock);
void handle_get_competition_by_id(const HttpRequest *req, uintptr_t client_sock);
void handle_create_competition(const HttpRequest *req, uintptr_t client_sock);
void handle_update_competition(const HttpRequest *req, uintptr_t client_sock);

/* Team Controllers */
void handle_get_teams(const HttpRequest *req, uintptr_t client_sock);
void handle_get_team_by_id(const HttpRequest *req, uintptr_t client_sock);
void handle_register_team(const HttpRequest *req, uintptr_t client_sock);

/* Match & Bracket Controllers */
void handle_get_matches(const HttpRequest *req, uintptr_t client_sock);
void handle_get_match_by_id(const HttpRequest *req, uintptr_t client_sock);
void handle_generate_fixtures(const HttpRequest *req, uintptr_t client_sock);
void handle_get_bracket(const HttpRequest *req, uintptr_t client_sock);
void handle_reschedule_match(const HttpRequest *req, uintptr_t client_sock);

/* Scorekeeping, Standings & Rollback Controllers */
void handle_submit_match_result(const HttpRequest *req, uintptr_t client_sock);
void handle_get_standings(const HttpRequest *req, uintptr_t client_sock);
void handle_get_history(const HttpRequest *req, uintptr_t client_sock);
void handle_undo_history(const HttpRequest *req, uintptr_t client_sock);

/* Dashboard, Venue & Settings Controllers */
void handle_get_dashboard_metrics(const HttpRequest *req, uintptr_t client_sock);
void handle_get_today_matches(const HttpRequest *req, uintptr_t client_sock);
void handle_get_venues(const HttpRequest *req, uintptr_t client_sock);
void handle_check_conflicts(const HttpRequest *req, uintptr_t client_sock);
void handle_export_backup(const HttpRequest *req, uintptr_t client_sock);
void handle_import_backup(const HttpRequest *req, uintptr_t client_sock);
void handle_reset_database(const HttpRequest *req, uintptr_t client_sock);

#endif /* MATCHMINT_CONTROLLERS_H */
