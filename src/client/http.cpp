#include "http.h"
#include "json.h"
#include "config.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int g_wsReady = 0;


int HttpInitialize(void)
{
    WSADATA wsaData;

    if (g_wsReady)
    {
        return 1;
    }

    if (WSAStartup(
        MAKEWORD(2, 2),
        &wsaData
    ) != 0)
    {
        return 0;
    }

    g_wsReady = 1;

    return 1;
}


void HttpShutdown(void)
{
    if (g_wsReady)
    {
        WSACleanup();
        g_wsReady = 0;
    }
}


/*
 * Parse a gateway URL of the form
 *   http://host:port
 *
 * host  - output buffer for the hostname
 * port  - output for the port number
 *
 * If no port is given, defaults to 8000.
 */
static int ParseGatewayUrl(
    const char *url,
    char *host,
    int hostSize,
    int *port
)
{
    const char *p;
    int len;
    int i;

    /*
     * Skip the scheme.
     */
    p = strstr(url, "://");

    if (p == NULL)
    {
        return 0;
    }

    p += 3;

    /*
     * Copy the host up to '/' or ':'.
     */
    i = 0;

    while (*p != '\0' &&
           *p != '/' &&
           *p != ':' &&
           i < hostSize - 1)
    {
        host[i++] = *p++;
    }

    host[i] = '\0';

    /*
     * Read the port if present.
     */
    if (*p == ':')
    {
        p++;

        *port = 0;

        while (isdigit((unsigned char)*p))
        {
            *port = *port * 10 + (*p - '0');
            p++;
        }
    }
    else
    {
        /*
         * Default port when none is
         * specified in the URL.
         */
        *port = 8000;
    }

    return 1;
}


/*
 * Build an HTTP request and send it
 * over a connected socket.
 */
static int SendRequest(
    SOCKET sock,
    const char *host,
    int port,
    const char *method,
    const char *path,
    const char *body,
    int bodyLen
)
{
    char header[1024];
    int headerLen;
    int total;
    int sent;

    headerLen = sprintf(
        header,
        "%s %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        method,
        path,
        host,
        port,
        bodyLen
    );

    total = headerLen + bodyLen;
    sent = 0;

    while (sent < total)
    {
        int chunk;
        int n;

        chunk = total - sent;

        if (chunk > 1024)
        {
            chunk = 1024;
        }

        if (sent < headerLen)
        {
            n = send(
                sock,
                header + sent,
                chunk > headerLen - sent
                    ? headerLen - sent
                    : chunk,
                0
            );
        }
        else
        {
            int offset = sent - headerLen;

            n = send(
                sock,
                body + offset,
                chunk > bodyLen - offset
                    ? bodyLen - offset
                    : chunk,
                0
            );
        }

        if (n <= 0)
        {
            return 0;
        }

        sent += n;
    }

    return 1;
}


/*
 * Read the full HTTP response body.
 *
 * The response may contain headers;
 * we skip them and return just the
 * body portion.
 */
static int ReadResponse(
    SOCKET sock,
    char *buf,
    int bufSize
)
{
    int total;
    int n;

    total = 0;

    while (total < bufSize - 1)
    {
        n = recv(
            sock,
            buf + total,
            bufSize - 1 - total,
            0
        );

        if (n <= 0)
        {
            break;
        }

        total += n;
    }

    buf[total] = '\0';

    return total;
}


/*
 * Skip the HTTP headers and return a
 * pointer to the body inside buf.
 */
static const char *GetBody(
    const char *buf
)
{
    const char *p;

    p = strstr(buf, "\r\n\r\n");

    if (p != NULL)
    {
        p += 4;
    }
    else
    {
        p = buf;
    }

    return p;
}


/*
 * Connect to the gateway and perform
 * a single HTTP exchange.
 */
static int DoHttpExchange(
    const char *gatewayUrl,
    const char *method,
    const char *path,
    const char *body,
    int bodyLen,
    char *response,
    int responseSize,
    char *statusOut,
    int statusSize
)
{
    char host[256];
    int port;
    struct sockaddr_in addr;
    struct hostent *hp;
    SOCKET sock;
    int bodyStart;
    const char *body;
    char raw[HTTP_RESPONSE_SIZE];
    int rawLen;

    if (!HttpInitialize())
    {
        strcpy(statusOut, "Winsock unavailable.");
        return 0;
    }

    if (!ParseGatewayUrl(gatewayUrl, host, sizeof(host), &port))
    {
        strcpy(statusOut, "Invalid gateway URL.");
        return 0;
    }

    /*
     * Resolve the hostname.
     */
    hp = gethostbyname(host);

    if (hp == NULL)
    {
        strcpy(statusOut, "Cannot resolve host.");
        return 0;
    }

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);

    {
        int i;

        for (i = 0;
             hp->h_addr_list[i] != NULL;
             i++)
        {
            memcpy(
                &addr.sin_addr,
                hp->h_addr_list[i],
                hp->h_length
            );

            break;
        }
    }

    sock = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (sock == INVALID_SOCKET)
    {
        strcpy(statusOut, "Socket error.");
        return 0;
    }

    if (connect(
        sock,
        (struct sockaddr *)&addr,
        sizeof(addr)
    ) != 0)
    {
        closesocket(sock);
        strcpy(statusOut, "Connection failed.");
        return 0;
    }

    if (bodyLen < 0)
    {
        bodyLen = 0;
    }

    if (!SendRequest(
        sock,
        host,
        port,
        method,
        path,
        (body != NULL && bodyLen > 0) ? body : "",
        bodyLen
    ))
    {
        closesocket(sock);
        strcpy(statusOut, "Send failed.");
        return 0;
    }

    rawLen = ReadResponse(sock, raw, sizeof(raw));

    closesocket(sock);

    if (rawLen <= 0)
    {
        strcpy(statusOut, "Empty response.");
        return 0;
    }

    body = GetBody(raw);

    /*
     * Copy the body into the caller's
     * buffer.
     */
    {
        int len;

        len = (int)strlen(body);

        if (len >= responseSize)
        {
            len = responseSize - 1;
        }

        memcpy(response, body, len);
        response[len] = '\0';
    }

    return 1;
}


/*
 * Send a chat request to the gateway
 * and extract the AI reply.
 */
int HttpChatRequest(
    const char *gatewayUrl,
    const char *userMessage,
    char *response,
    int responseSize,
    char *statusOut,
    int statusSize
)
{
    char jsonBody[4096];
    char rawBody[HTTP_RESPONSE_SIZE];

    if (responseSize <= 0)
    {
        return 0;
    }

    response[0] = '\0';

    if (!JsonBuildRequest(
        userMessage,
        jsonBody,
        sizeof(jsonBody)
    ))
    {
        strcpy(statusOut, "JSON build failed.");
        return 0;
    }

    if (!DoHttpExchange(
        gatewayUrl,
        "POST",
        "/v1/chat",
        jsonBody,
        (int)strlen(jsonBody),
        rawBody,
        sizeof(rawBody),
        statusOut,
        statusSize
    ))
    {
        return 0;
    }

    if (!JsonExtractResponse(
        rawBody,
        response,
        responseSize
    ))
    {
        /*
         * The gateway did not return
         * a "response" field.  Return
         * the raw body so the user
         * can at least see something.
         */
        strncpy(response, rawBody, responseSize - 1);
        response[responseSize - 1] = '\0';

        strcpy(statusOut, "Unexpected gateway reply.");

        return 0;
    }

    strcpy(statusOut, "");

    return 1;
}


/*
 * Test the gateway connection.
 */
int HttpTestConnection(
    const char *gatewayUrl,
    char *statusOut,
    int statusSize
)
{
    char rawBody[256];

    if (statusSize > 0)
    {
        statusOut[0] = '\0';
    }

    if (!DoHttpExchange(
        gatewayUrl,
        "GET",
        "/",
        NULL,
        0,
        rawBody,
        sizeof(rawBody),
        statusOut,
        statusSize
    ))
    {
        return 0;
    }

    strcpy(statusOut, "Gateway reachable.");

    return 1;
}
