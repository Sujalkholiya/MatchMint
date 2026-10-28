#ifndef MATCHMINT_JSON_UTIL_H
#define MATCHMINT_JSON_UTIL_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} JsonBuffer;

JsonBuffer *json_buffer_create(void);
void json_buffer_free(JsonBuffer *jb);
void json_buffer_append(JsonBuffer *jb, const char *str);
void json_buffer_append_printf(JsonBuffer *jb, const char *fmt, ...);

bool json_get_string(const char *json, const char *key, char *out_val, size_t max_len);
bool json_get_int(const char *json, const char *key, int *out_val);
bool json_get_bool(const char *json, const char *key, bool *out_val);

void json_escape_string(const char *in, char *out, size_t max_len);

#endif /* MATCHMINT_JSON_UTIL_H */
