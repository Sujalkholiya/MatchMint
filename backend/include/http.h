#ifndef MATCHMINT_HTTP_H
#define MATCHMINT_HTTP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define HTTP_MAX_METHOD_LEN 16
#define HTTP_MAX_PATH_LEN 256
#define HTTP_MAX_QUERY_LEN 512
#define HTTP_MAX_HEADERS 32
#define HTTP_MAX_PARAMS 8
#define HTTP_PARAM_KEY_LEN 32
#define HTTP_PARAM_VAL_LEN 64

typedef struct {
    char key[HTTP_PARAM_KEY_LEN];
    char value[HTTP_PARAM_VAL_LEN];
} HttpPathParam;

typedef struct {
    char method[HTTP_MAX_METHOD_LEN]; // "GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"
    char raw_url[HTTP_MAX_PATH_LEN + HTTP_MAX_QUERY_LEN];
    char path[HTTP_MAX_PATH_LEN];     // e.g. "/api/v1/competitions"
    char query[HTTP_MAX_QUERY_LEN];   // e.g. "category=sports&status=live"
    char content_type[128];
    char authorization[512];          // Bearer token if present
    size_t content_length;
    char *body;                       // dynamically allocated or buffered
    HttpPathParam params[HTTP_MAX_PARAMS];
    int param_count;
} HttpRequest;

typedef struct {
    int status_code;
    char content_type[64];
    char *body;
    size_t body_len;
} HttpResponse;

/* Helper to extract query parameter value by key */
bool http_get_query_param(const char *query_string, const char *key, char *out_val, size_t max_len);

/* Helper to get path parameter value (e.g. from :id) */
const char *http_get_path_param(const HttpRequest *req, const char *key);

#endif /* MATCHMINT_HTTP_H */
