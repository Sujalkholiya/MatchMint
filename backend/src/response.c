#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#define closesocket close
typedef int SOCKET;
#endif

static const char *get_status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

void response_send_cors_options(uintptr_t client_sock) {
    SOCKET sock = (SOCKET)client_sock;
    const char *header =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

    send(sock, header, (int)strlen(header), 0);
}

void response_send_json(uintptr_t client_sock, int status_code, const char *json_string) {
    SOCKET sock = (SOCKET)client_sock;
    size_t body_len = json_string ? strlen(json_string) : 0;
    const char *status_text = get_status_text(status_code);

    char header[512];
    int header_len = snprintf(
        header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_text, body_len
    );

    send(sock, header, header_len, 0);
    if (body_len > 0) {
        send(sock, json_string, (int)body_len, 0);
    }
}

void response_send_success(uintptr_t client_sock, int status_code, const char *message, const char *json_data) {
    char *buf = (char *)malloc(strlen(message ? message : "") + strlen(json_data ? json_data : "{}") + 256);
    if (!buf) return;

    if (json_data && strlen(json_data) > 0) {
        sprintf(buf, "{\"success\":true,\"message\":\"%s\",\"data\":%s}", message ? message : "Success", json_data);
    } else {
        sprintf(buf, "{\"success\":true,\"message\":\"%s\",\"data\":null}", message ? message : "Success");
    }

    response_send_json(client_sock, status_code, buf);
    free(buf);
}

void response_send_error(uintptr_t client_sock, int status_code, const char *error_code, const char *message) {
    char buf[512];
    snprintf(
        buf, sizeof(buf),
        "{\"success\":false,\"error\":{\"code\":\"%s\",\"message\":\"%s\",\"details\":[]}}",
        error_code ? error_code : "ERROR",
        message ? message : "An error occurred"
    );
    response_send_json(client_sock, status_code, buf);
}
