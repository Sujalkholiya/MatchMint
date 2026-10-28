#include "router.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RouteEntry g_routes[ROUTER_MAX_ROUTES];
static int g_route_count = 0;

void router_init(void) {
    g_route_count = 0;
    memset(g_routes, 0, sizeof(g_routes));
}

bool router_add(const char *method, const char *pattern, RouteHandler handler) {
    if (g_route_count >= ROUTER_MAX_ROUTES || !method || !pattern || !handler) {
        return false;
    }

    RouteEntry *entry = &g_routes[g_route_count++];
    snprintf(entry->method, sizeof(entry->method), "%s", method);
    snprintf(entry->pattern, sizeof(entry->pattern), "%s", pattern);
    entry->handler = handler;
    return true;
}

static bool match_segments(const char *pattern, const char *path, HttpRequest *req) {
    char pat_buf[HTTP_MAX_PATH_LEN];
    char path_buf[HTTP_MAX_PATH_LEN];
    snprintf(pat_buf, sizeof(pat_buf), "%s", pattern);
    snprintf(path_buf, sizeof(path_buf), "%s", path);

    // Strip trailing slashes for consistency
    size_t pat_len = strlen(pat_buf);
    if (pat_len > 1 && pat_buf[pat_len - 1] == '/') pat_buf[pat_len - 1] = '\0';
    size_t path_len = strlen(path_buf);
    if (path_len > 1 && path_buf[path_len - 1] == '/') path_buf[path_len - 1] = '\0';

    char *pat_save = NULL;
    char *path_save = NULL;
    char *pat_tok = strtok_r(pat_buf, "/", &pat_save);
    char *path_tok = strtok_r(path_buf, "/", &path_save);

    req->param_count = 0;

    while (pat_tok && path_tok) {
        if (pat_tok[0] == ':') {
            // Path parameter wildcard!
            if (req->param_count < HTTP_MAX_PARAMS) {
                snprintf(req->params[req->param_count].key, sizeof(req->params[0].key), "%s", pat_tok + 1);
                snprintf(req->params[req->param_count].value, sizeof(req->params[0].value), "%s", path_tok);
                req->param_count++;
            }
        } else if (strcmp(pat_tok, path_tok) != 0) {
            return false;
        }

        pat_tok = strtok_r(NULL, "/", &pat_save);
        path_tok = strtok_r(NULL, "/", &path_save);
    }

    // Both must finish at the same time
    return (pat_tok == NULL && path_tok == NULL);
}

const char *http_get_path_param(const HttpRequest *req, const char *key) {
    if (!req || !key) return NULL;
    for (int i = 0; i < req->param_count; i++) {
        if (strcmp(req->params[i].key, key) == 0) {
            return req->params[i].value;
        }
    }
    return NULL;
}

bool http_get_query_param(const char *query_string, const char *key, char *out_val, size_t max_len) {
    if (!query_string || !key || !out_val || max_len == 0) return false;

    char q_buf[HTTP_MAX_QUERY_LEN];
    snprintf(q_buf, sizeof(q_buf), "%s", query_string);

    char *saveptr = NULL;
    char *pair = strtok_r(q_buf, "&", &saveptr);

    while (pair) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(pair, key) == 0) {
                snprintf(out_val, max_len, "%s", eq + 1);
                return true;
            }
        } else {
            if (strcmp(pair, key) == 0) {
                out_val[0] = '\0';
                return true;
            }
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }

    return false;
}

void router_dispatch(HttpRequest *req, uintptr_t client_sock) {
    if (!req) {
        response_send_error(client_sock, 400, "BAD_REQUEST", "Invalid request payload");
        return;
    }

    // 1. Automatic preflight CORS handler
    if (strcmp(req->method, "OPTIONS") == 0) {
        response_send_cors_options(client_sock);
        return;
    }

    // 2. Route matching
    for (int i = 0; i < g_route_count; i++) {
        if (strcmp(g_routes[i].method, req->method) == 0) {
            if (match_segments(g_routes[i].pattern, req->path, req)) {
                g_routes[i].handler(req, client_sock);
                return;
            }
        }
    }

    // 3. No route matched
    char err_msg[512];
    snprintf(err_msg, sizeof(err_msg), "Cannot %s %s", req->method, req->path);
    response_send_error(client_sock, 404, "NOT_FOUND", err_msg);
}
