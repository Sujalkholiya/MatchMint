#ifndef MATCHMINT_SERVER_H
#define MATCHMINT_SERVER_H

#include <stdbool.h>

/**
 * Initializes the HTTP server listening on the specified port.
 *
 * @param port Port number (e.g. 8080)
 * @return 0 on success, non-zero on error.
 */
int server_init(int port);

/**
 * Processes one incoming HTTP client request with a timeout.
 *
 * @param timeout_ms Timeout in milliseconds
 * @return 1 if a request was handled, 0 on timeout, -1 on error.
 */
int server_poll_and_process(int timeout_ms);

/**
 * Starts the server loop.
 *
 * @param should_run Callback function returning false when server should terminate.
 */
void server_start_loop(bool (*should_run)(void));

/**
 * Closes the server socket and cleans up Winsock/network resources.
 */
void server_close(void);

#endif /* MATCHMINT_SERVER_H */
