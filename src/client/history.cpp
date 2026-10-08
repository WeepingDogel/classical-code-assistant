#include "history.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>


/*
 * Compatibility definition.
 *
 * INVALID_FILE_ATTRIBUTES is not
 * available on old MinGW / Dev-C++.
 * We define it here so that the
 * project can be built with the
 * same toolchains it targets.
 */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES \
    ((DWORD)-1)
#endif


static char g_logDir[MAX_PATH];
static int g_logDirReady = 0;


/*
 * Locate the chatlog directory next to
 * the executable and make sure it
 * exists.
 *
 * Uses the same GetModuleFileNameA +
 * strrchr('\\') trick as config.cpp
 * for compatibility with old MinGW.
 */
int HistoryEnsureDir(void)
{
    char exePath[MAX_PATH];
    char *slash;

    if (g_logDirReady)
    {
        return 1;
    }

    GetModuleFileNameA(
        NULL,
        exePath,
        sizeof(exePath)
    );

    slash = strrchr(
        exePath,
        '\\'
    );

    if (slash != NULL)
    {
        *(slash + 1) = '\0';
    }

    wsprintfA(
        g_logDir,
        "%schatlog\\",
        exePath
    );

    /*
     * CreateDirectoryA returns 0 on
     * failure; the common "already
     * exists" case is not an error
     * for our purposes.
     */
    if (!CreateDirectoryA(
        g_logDir,
        NULL
    ))
    {
        DWORD err = GetLastError();

        if (err != ERROR_ALREADY_EXISTS)
        {
            return 0;
        }
    }

    g_logDirReady = 1;

    return 1;
}


/*
 * Build the full path to a provider's
 * log file and return it through a
 * static buffer.
 */
const char *HistoryFileFor(
    int providerIndex
)
{
    static char path[MAX_PATH];
    char base[MAX_PATH];

    HistoryEnsureDir();

    wsprintfA(
        base,
        "%sProvider%d.txt",
        g_logDir,
        providerIndex
    );

    strcpy(
        path,
        base
    );

    return path;
}


/*
 * Check whether a log file already
 * exists for a provider.
 */
int HistoryExists(
    int providerIndex
)
{
    const char *path;

    if (providerIndex < 0)
    {
        return 0;
    }

    path = HistoryFileFor(providerIndex);

    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    return 1;
}


/*
 * Append one user/AI exchange to the
 * log for a provider.
 *
 * Each exchange produces two timestamped
 * lines so that the file stays
 * readable in Notepad on Windows 2000.
 */
void HistoryAppend(
    int providerIndex,
    const char *userText,
    const char *aiText
)
{
    const char *path;
    HANDLE hFile;
    SYSTEMTIME st;
    char line[4096];
    DWORD written;

    if (providerIndex < 0)
    {
        return;
    }

    if (!HistoryEnsureDir())
    {
        return;
    }

    path = HistoryFileFor(providerIndex);

    hFile = CreateFileA(
        path,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    /*
     * Move to end of file so we
     * append instead of overwrite.
     */
    SetFilePointer(
        hFile,
        0,
        NULL,
        FILE_END
    );

    GetLocalTime(&st);

    /*
     * The user line.  We keep a
     * simple "USER:" / "AI:" tag
     * so the format stays trivial.
     */
    {
        int n;

        n = sprintf(
            line,
            "[%04d-%02d-%02d %02d:%02d:%02d] USER: %s\r\n",
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond,
            (userText != NULL) ? userText : ""
        );

        if (n > 0)
        {
            WriteFile(
                hFile,
                line,
                (DWORD)n,
                &written,
                NULL
            );
        }
    }

    /*
     * The AI line.
     */
    {
        int n;

        n = sprintf(
            line,
            "[%04d-%02d-%02d %02d:%02d:%02d] AI: %s\r\n",
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond,
            (aiText != NULL) ? aiText : ""
        );

        if (n > 0)
        {
            WriteFile(
                hFile,
                line,
                (DWORD)n,
                &written,
                NULL
            );
        }
    }

    CloseHandle(hFile);
}


/*
 * Read a log file into a buffer.
 *
 * Capped at maxBytes characters; the
 * caller is responsible for supplying
 * enough room.  The result is always
 * NUL-terminated on success.
 */
int HistoryLoad(
    const char *path,
    char *buffer,
    int maxBytes
)
{
    HANDLE hFile;
    DWORD available;
    DWORD bytesToRead;
    DWORD read;
    DWORD offset;

    if (buffer == NULL ||
        maxBytes <= 0 ||
        path == NULL)
    {
        return 0;
    }

    buffer[0] = '\0';

    hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    available = GetFileSize(hFile, NULL);

    if (available == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return 0;
    }

    /*
     * Do not read more than the
     * caller's buffer allows.
     * Leave one byte for NUL.
     */
    bytesToRead = available;

    if (bytesToRead >= (DWORD)maxBytes)
    {
        bytesToRead = (DWORD)maxBytes - 1;
    }

    offset = 0;

    while (offset < bytesToRead)
    {
        if (!ReadFile(
            hFile,
            buffer + offset,
            bytesToRead - offset,
            &read,
            NULL
        ))
        {
            break;
        }

        if (read == 0)
        {
            break;
        }

        offset += read;
    }

    buffer[offset] = '\0';

    CloseHandle(hFile);

    return 1;
}