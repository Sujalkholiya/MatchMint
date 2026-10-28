#include "json_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

JsonBuffer *json_buffer_create(void) {
    JsonBuffer *jb = (JsonBuffer *)malloc(sizeof(JsonBuffer));
    if (!jb) return NULL;
    jb->capacity = 1024;
    jb->length = 0;
    jb->data = (char *)malloc(jb->capacity);
    if (!jb->data) {
        free(jb);
        return NULL;
    }
    jb->data[0] = '\0';
    return jb;
}

void json_buffer_free(JsonBuffer *jb) {
    if (!jb) return;
    if (jb->data) free(jb->data);
    free(jb);
}

void json_buffer_append(JsonBuffer *jb, const char *str) {
    if (!jb || !str) return;
    size_t len = strlen(str);
    while (jb->length + len + 1 >= jb->capacity) {
        jb->capacity *= 2;
        char *new_data = (char *)realloc(jb->data, jb->capacity);
        if (!new_data) return;
        jb->data = new_data;
    }
    memcpy(jb->data + jb->length, str, len);
    jb->length += len;
    jb->data[jb->length] = '\0';
}

void json_buffer_append_printf(JsonBuffer *jb, const char *fmt, ...) {
    if (!jb || !fmt) return;
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    json_buffer_append(jb, buf);
}

void json_escape_string(const char *in, char *out, size_t max_len) {
    if (!in || !out || max_len == 0) return;
    size_t out_idx = 0;
    for (size_t i = 0; in[i] != '\0' && out_idx + 2 < max_len; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '"') {
            out[out_idx++] = '\\';
            out[out_idx++] = '"';
        } else if (c == '\\') {
            out[out_idx++] = '\\';
            out[out_idx++] = '\\';
        } else if (c == '\n') {
            out[out_idx++] = '\\';
            out[out_idx++] = 'n';
        } else if (c == '\r') {
            out[out_idx++] = '\\';
            out[out_idx++] = 'r';
        } else if (c == '\t') {
            out[out_idx++] = '\\';
            out[out_idx++] = 't';
        } else {
            out[out_idx++] = (char)c;
        }
    }
    out[out_idx] = '\0';
}

bool json_get_string(const char *json, const char *key, char *out_val, size_t max_len) {
    if (!json || !key || !out_val || max_len == 0) return false;

    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return false;

    pos += strlen(pattern);
    while (*pos == ' ' || *pos == ':') pos++;

    if (*pos != '"') return false;
    pos++; // skip opening quote

    size_t idx = 0;
    while (*pos != '\0' && *pos != '"' && idx + 1 < max_len) {
        if (*pos == '\\' && *(pos + 1) != '\0') {
            pos++; // handle escaped character
        }
        out_val[idx++] = *pos++;
    }
    out_val[idx] = '\0';
    return true;
}

bool json_get_int(const char *json, const char *key, int *out_val) {
    if (!json || !key || !out_val) return false;

    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return false;

    pos += strlen(pattern);
    while (*pos == ' ' || *pos == ':') pos++;

    char *endptr = NULL;
    long val = strtol(pos, &endptr, 10);
    if (endptr == pos) return false;

    *out_val = (int)val;
    return true;
}

bool json_get_bool(const char *json, const char *key, bool *out_val) {
    if (!json || !key || !out_val) return false;

    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return false;

    pos += strlen(pattern);
    while (*pos == ' ' || *pos == ':') pos++;

    if (strncmp(pos, "true", 4) == 0) {
        *out_val = true;
        return true;
    } else if (strncmp(pos, "false", 5) == 0) {
        *out_val = false;
        return true;
    }

    return false;
}
