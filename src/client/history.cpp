#include "history.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>


/*
 * Compatibility definition.
 *
 * INVALID_FILE_ATTRIBUTES is not
 * available on old MinGW / Dev-C++.
 */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES \
    ((DWORD)-1)
#endif


static char g_logDir[MAX_PATH];
static int g_logDirReady = 0;


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


const char *HistoryFilePath(void)
{
    static char path[MAX_PATH];

    HistoryEnsureDir();

    wsprintfA(
        path,
        "%schatlog.txt",
        g_logDir
    );

    return path;
}


int HistoryExists(void)
{
    const char *path;

    if (!HistoryEnsureDir())
    {
        return 0;
    }

    path = HistoryFilePath();

    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    return 1;
}


void HistoryAppend(
    const char *userText,
    const char *aiText
)
{
    const char *path;
    HANDLE hFile;
    SYSTEMTIME st;
    char line[4096];
    DWORD written;

    if (!HistoryEnsureDir())
    {
        return;
    }

    path = HistoryFilePath();

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

    SetFilePointer(
        hFile,
        0,
        NULL,
        FILE_END
    );

    GetLocalTime(&st);

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