#ifndef MATCHMINT_RESPONSE_H
#define MATCHMINT_RESPONSE_H

#include <stdint.h>
#include <stddef.h>

void response_send_cors_options(uintptr_t client_sock);

void response_send_json(uintptr_t client_sock, int status_code, const char *json_string);

void response_send_success(uintptr_t client_sock, int status_code, const char *message, const char *json_data);

void response_send_error(uintptr_t client_sock, int status_code, const char *error_code, const char *message);

#endif /* MATCHMINT_RESPONSE_H */
