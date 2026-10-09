#ifndef CLASSICAL_HTTP_H
#define CLASSICAL_HTTP_H

/*
 * Compatibility with old MinGW / Dev-C++ toolchains.
 *
 * The system winsock2.h shipped with old Dev-C++ contains an
 * unbalanced #endif (GCC reports it around line 46) that aborts
 * the build.  Instead of pulling in that broken header, this
 * project ships its own minimal, ABI-correct shim at
 *   src/client/winsock2_compat.h
 * which declares only the Winsock types and functions the HTTP
 * layer uses.  It is included via a relative path so the system
 * header is never reached.
 *
 * WIN32_LEAN_AND_MEAN must be defined before <windows.h> is
 * included so that windows.h does not pull in winsock.h
 * (which conflicts with the SOCKET type in the shim).
 *
 * Link with:  -lws2_32
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "winsock2_compat.h"


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