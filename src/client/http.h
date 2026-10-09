#ifndef CLASSICAL_HTTP_H
#define CLASSICAL_HTTP_H

#include <winsock2.h>


/*
 * Size of the response buffer used by
 * the HTTP client.  Must be large enough
 * to hold a typical AI reply plus the
 * HTTP headers.
 */
#define HTTP_RESPONSE_SIZE  8192


/*
 * Send a POST /v1/chat request to the
 * gateway and extract the AI reply.
 *
 * gatewayUrl  - e.g. "http://192.168.1.100:8000"
 * userMessage - the user's text
 * response    - output buffer for the AI reply
 * responseSize - size of response
 * statusOut   - human-readable status on failure
 * statusSize  - size of statusOut
 *
 * Returns TRUE on success, FALSE on
 * failure.  On failure, statusOut
 * contains a short description.
 */
int HttpChatRequest(
    const char *gatewayUrl,
    const char *userMessage,
    char *response,
    int responseSize,
    char *statusOut,
    int statusSize
);


/*
 * Test whether the gateway is reachable.
 *
 * Sends a GET / to the gateway.
 *
 * Returns TRUE if a response is
 * received, FALSE otherwise.
 */
int HttpTestConnection(
    const char *gatewayUrl,
    char *statusOut,
    int statusSize
);


/*
 * Initialize Winsock.  Must be called
 * before any Http* function.
 *
 * Returns TRUE on success.
 */
int HttpInitialize(void);


/*
 * Shut down Winsock.
 */
void HttpShutdown(void);


#endif