#include "server.h"
#include "http.h"
#include "router.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCK INVALID_SOCKET
#define SOCK_ERR SOCKET_ERROR
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
typedef int socket_t;
#define INVALID_SOCK (-1)
#define SOCK_ERR (-1)
#define closesocket close
#endif

static socket_t g_server_fd = INVALID_SOCK;
static int g_server_port = 8080;

static void parse_http_request(char *raw_req, size_t raw_len, HttpRequest *req) {
    (void)raw_len;
    memset(req, 0, sizeof(HttpRequest));

    char *body_sep = strstr(raw_req, "\r\n\r\n");
    char *body_ptr = NULL;
    if (body_sep) {
        *body_sep = '\0';
        body_ptr = body_sep + 4;
    } else {
        body_sep = strstr(raw_req, "\n\n");
        if (body_sep) {
            *body_sep = '\0';
            body_ptr = body_sep + 2;
        }
    }

    char *saveptr = NULL;
    char *line = strtok_r(raw_req, "\r\n", &saveptr);
    if (!line) return;

    // 1. Request Line: METHOD URI PROTOCOL
    char method[16] = {0};
    char uri[HTTP_MAX_PATH_LEN] = {0};
    char proto[16] = {0};

    if (sscanf(line, "%15s %255s %15s", method, uri, proto) >= 2) {
        snprintf(req->method, sizeof(req->method), "%s", method);
        snprintf(req->raw_url, sizeof(req->raw_url), "%s", uri);

        char *q = strchr(uri, '?');
        if (q) {
            *q = '\0';
            snprintf(req->path, sizeof(req->path), "%s", uri);
            snprintf(req->query, sizeof(req->query), "%s", q + 1);
        } else {
            snprintf(req->path, sizeof(req->path), "%s", uri);
            req->query[0] = '\0';
        }
    }

    // 2. Headers
    line = strtok_r(NULL, "\r\n", &saveptr);
    while (line && strlen(line) > 0) {
        if (strncasecmp(line, "Content-Type:", 13) == 0) {
            const char *val = line + 13;
            while (*val == ' ') val++;
            snprintf(req->content_type, sizeof(req->content_type), "%s", val);
        } else if (strncasecmp(line, "Content-Length:", 15) == 0) {
            req->content_length = (size_t)atoi(line + 15);
        } else if (strncasecmp(line, "Authorization:", 14) == 0) {
            const char *val = line + 14;
            while (*val == ' ') val++;
            snprintf(req->authorization, sizeof(req->authorization), "%s", val);
        }

        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    // 3. Body
    if (body_ptr && strlen(body_ptr) > 0) {
        req->body = strdup(body_ptr);
        if (req->content_length == 0) {
            req->content_length = strlen(body_ptr);
        }
    } else {
        req->body = NULL;
    }
}

int server_init(int port) {
    g_server_port = port;

#if defined(_WIN32)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[SERVER ERROR] WSAStartup failed.\n");
        return -1;
    }
#endif

    g_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_server_fd == INVALID_SOCK) {
        fprintf(stderr, "[SERVER ERROR] Failed to create socket.\n");
        return -1;
    }

    int opt = 1;
#if defined(_WIN32)
    setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t)port);

    if (bind(g_server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCK_ERR) {
        fprintf(stderr, "[SERVER ERROR] Failed to bind to port %d.\n", port);
        closesocket(g_server_fd);
        g_server_fd = INVALID_SOCK;
        return -1;
    }

    if (listen(g_server_fd, 64) == SOCK_ERR) {
        fprintf(stderr, "[SERVER ERROR] Failed to listen on socket.\n");
        closesocket(g_server_fd);
        g_server_fd = INVALID_SOCK;
        return -1;
    }

    printf("[INFO] MatchMint HTTP Server listening on http://0.0.0.0:%d\n", port);
    return 0;
}

int server_poll_and_process(int timeout_ms) {
    if (g_server_fd == INVALID_SOCK) return -1;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(g_server_fd, &readfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int activity = select((int)(g_server_fd + 1), &readfds, NULL, NULL, &tv);
    if (activity <= 0) {
        return activity; // 0 = timeout, < 0 = error
    }

    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    socket_t client_sock = accept(g_server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_sock == INVALID_SOCK) {
        return -1;
    }

    // Set receive timeout on client socket (2 seconds)
#if defined(_WIN32)
    DWORD sock_tv = 2000;
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&sock_tv, sizeof(sock_tv));
#else
    struct timeval sock_tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &sock_tv, sizeof(sock_tv));
#endif

    char buffer[65536]; // 64 KB buffer for incoming request
    int bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        // Handle Expect: 100-continue from .NET / curl if present
        if (strstr(buffer, "100-continue") || strstr(buffer, "100-Continue")) {
            send(client_sock, "HTTP/1.1 100 Continue\r\n\r\n", 25, 0);
        }

        // Ensure complete body is read based on Content-Length
        char *sep = strstr(buffer, "\r\n\r\n");
        if (!sep) sep = strstr(buffer, "\n\n");
        if (sep) {
            int header_len = (int)((sep - buffer) + (strncmp(sep, "\r\n\r\n", 4) == 0 ? 4 : 2));
            char *cl = strstr(buffer, "Content-Length:");
            if (!cl) cl = strstr(buffer, "content-length:");
            if (cl) {
                int content_len = atoi(cl + 15);
                int body_so_far = bytes_read - header_len;
                while (body_so_far < content_len && bytes_read < (int)sizeof(buffer) - 1) {
                    int more = recv(client_sock, buffer + bytes_read, sizeof(buffer) - 1 - bytes_read, 0);
                    if (more <= 0) break;
                    bytes_read += more;
                    body_so_far += more;
                    buffer[bytes_read] = '\0';
                }
            }
        }

        HttpRequest req;
        parse_http_request(buffer, (size_t)bytes_read, &req);

        // Dispatch via router
        router_dispatch(&req, (uintptr_t)client_sock);

        if (req.body) {
            free(req.body);
            req.body = NULL;
        }
    }

    closesocket(client_sock);
    return 1;
}

void server_start_loop(bool (*should_run)(void)) {
    while (should_run == NULL || should_run()) {
        server_poll_and_process(500);
    }
}

void server_close(void) {
    if (g_server_fd != INVALID_SOCK) {
        closesocket(g_server_fd);
        g_server_fd = INVALID_SOCK;
    }
#if defined(_WIN32)
    WSACleanup();
#endif
    printf("[INFO] MatchMint HTTP Server stopped.\n");
}
