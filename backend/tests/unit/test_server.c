#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "http.h"
#include "router.h"
#include "response.h"
#include "json_util.h"

static bool g_health_called = false;
static char g_last_comp_id[64] = {0};

static void dummy_health_handler(const HttpRequest *req, uintptr_t client_sock) {
    (void)client_sock;
    g_health_called = true;
}

static void dummy_comp_handler(const HttpRequest *req, uintptr_t client_sock) {
    (void)client_sock;
    const char *id = http_get_path_param(req, "id");
    if (id) {
        snprintf(g_last_comp_id, sizeof(g_last_comp_id), "%s", id);
    }
}

static void test_router_and_params(void) {
    printf("[TEST] 1. Router Path Pattern & Parameter Extraction...\n");

    router_init();
    assert(router_add("GET", "/api/v1/health", dummy_health_handler));
    assert(router_add("GET", "/api/v1/competitions/:id", dummy_comp_handler));

    HttpRequest req1;
    memset(&req1, 0, sizeof(req1));
    snprintf(req1.method, sizeof(req1.method), "GET");
    snprintf(req1.path, sizeof(req1.path), "/api/v1/health");

    g_health_called = false;
    router_dispatch(&req1, 0);
    assert(g_health_called == true);

    HttpRequest req2;
    memset(&req2, 0, sizeof(req2));
    snprintf(req2.method, sizeof(req2.method), "GET");
    snprintf(req2.path, sizeof(req2.path), "/api/v1/competitions/comp-gehu-2026");

    memset(g_last_comp_id, 0, sizeof(g_last_comp_id));
    router_dispatch(&req2, 0);
    assert(strcmp(g_last_comp_id, "comp-gehu-2026") == 0);

    printf("  -> Router and parameter extraction PASSED.\n");
}

static void test_query_params(void) {
    printf("[TEST] 2. Query Parameter Extraction...\n");

    const char *qs = "category=sports&format=single_elimination&limit=10";
    char val[64];

    assert(http_get_query_param(qs, "category", val, sizeof(val)) == true);
    assert(strcmp(val, "sports") == 0);

    assert(http_get_query_param(qs, "format", val, sizeof(val)) == true);
    assert(strcmp(val, "single_elimination") == 0);

    assert(http_get_query_param(qs, "limit", val, sizeof(val)) == true);
    assert(strcmp(val, "10") == 0);

    assert(http_get_query_param(qs, "nonexistent", val, sizeof(val)) == false);

    printf("  -> Query parameter extraction PASSED.\n");
}

static void test_json_utilities(void) {
    printf("[TEST] 3. JSON Utility Serialization & Parsing...\n");

    const char *sample_json = "{\"name\":\"GEHU Winter Cup 2026\",\"totalTeams\":8,\"isLive\":true}";

    char str_val[64];
    int int_val = 0;
    bool bool_val = false;

    assert(json_get_string(sample_json, "name", str_val, sizeof(str_val)) == true);
    assert(strcmp(str_val, "GEHU Winter Cup 2026") == 0);

    assert(json_get_int(sample_json, "totalTeams", &int_val) == true);
    assert(int_val == 8);

    assert(json_get_bool(sample_json, "isLive", &bool_val) == true);
    assert(bool_val == true);

    JsonBuffer *jb = json_buffer_create();
    assert(jb != NULL);
    json_buffer_append(jb, "{\"status\":\"ok\"");
    json_buffer_append_printf(jb, ",\"port\":%d}", 8080);
    assert(strcmp(jb->data, "{\"status\":\"ok\",\"port\":8080}") == 0);
    json_buffer_free(jb);

    printf("  -> JSON utilities PASSED.\n");
}

int main(void) {
    printf("=====================================================\n");
    printf("  MatchMint HTTP Server & Router Test Suite\n");
    printf("=====================================================\n");

    test_router_and_params();
    test_query_params();
    test_json_utilities();

    printf("=====================================================\n");
    printf("  ALL HTTP & ROUTER TESTS PASSED (100%%)\n");
    printf("=====================================================\n");
    return 0;
}
