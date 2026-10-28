#ifndef MATCHMINT_ROUTER_H
#define MATCHMINT_ROUTER_H

#include "http.h"
#include <stdint.h>

typedef void (*RouteHandler)(const HttpRequest *req, uintptr_t client_sock);

typedef struct {
    char method[HTTP_MAX_METHOD_LEN];
    char pattern[HTTP_MAX_PATH_LEN];
    RouteHandler handler;
} RouteEntry;

#define ROUTER_MAX_ROUTES 64

/**
 * Initializes the router table.
 */
void router_init(void);

/**
 * Registers a route handler for a given HTTP method and path pattern.
 * e.g., router_add("GET", "/api/v1/competitions/:id", handler);
 */
bool router_add(const char *method, const char *pattern, RouteHandler handler);

/**
 * Dispatches an incoming HTTP request to the matching handler.
 * Automatically handles CORS OPTIONS preflight.
 */
void router_dispatch(HttpRequest *req, uintptr_t client_sock);

#endif /* MATCHMINT_ROUTER_H */
