#include "json.h"

#include <stdio.h>
#include <string.h>


int JsonBuildRequest(
    const char *userMessage,
    char *outBuf,
    int outSize
)
{
    int pos;

    if (userMessage == NULL ||
        outBuf == NULL ||
        outSize < 4)
    {
        return 0;
    }

    pos = 0;

    {
        const char *prefix = "{\"message\":\"";
        int len = (int)strlen(prefix);

        if (outSize - 1 < len)
        {
            return 0;
        }

        while (len > 0)
        {
            outBuf[pos++] = *prefix++;
            len--;
        }
    }

    while (*userMessage != '\0')
    {
        char c = *userMessage;

        if (outSize - 1 < pos + 2)
        {
            return 0;
        }

        if (c == '\\')
        {
            outBuf[pos++] = '\\';
            outBuf[pos++] = '\\';
        }
        else if (c == '"')
        {
            outBuf[pos++] = '\\';
            outBuf[pos++] = '"';
        }
        else if (c == '\n')
        {
            outBuf[pos++] = '\\';
            outBuf[pos++] = 'n';
        }
        else if (c == '\r')
        {
            outBuf[pos++] = '\\';
            outBuf[pos++] = 'r';
        }
        else if (c == '\t')
        {
            outBuf[pos++] = '\\';
            outBuf[pos++] = 't';
        }
        else
        {
            outBuf[pos++] = c;
        }

        userMessage++;
    }

    if (outSize - 1 < pos + 2)
    {
        return 0;
    }

    outBuf[pos++] = '"';
    outBuf[pos++] = '}';
    outBuf[pos] = '\0';

    return 1;
}


int JsonExtractResponse(
    const char *json,
    char *outBuf,
    int outSize
)
{
    const char *marker = "\"response\":\"";
    const char *start;
    int pos;

    if (json == NULL ||
        outBuf == NULL ||
        outSize < 2)
    {
        return 0;
    }

    start = strstr(json, marker);

    if (start == NULL)
    {
        return 0;
    }

    start += (int)strlen(marker);

    pos = 0;

    while (*start != '\0')
    {
        char c = *start;

        if (c == '"')
        {
            break;
        }

        if (c == '\\' && start[1] != '\0')
        {
            char esc = start[1];

            if (esc == 'n')
            {
                c = '\n';
            }
            else if (esc == 'r')
            {
                c = '\r';
            }
            else if (esc == 't')
            {
                c = '\t';
            }
            else if (esc == '"')
            {
                c = '"';
            }
            else if (esc == '\\')
            {
                c = '\\';
            }

            start += 2;
        }
        else
        {
            start += 1;
        }

        if (outSize - 1 < pos + 1)
        {
            return 0;
        }

        outBuf[pos++] = c;
    }

    outBuf[pos] = '\0';

    if (pos == 0)
    {
        return 0;
    }

    return 1;
}
