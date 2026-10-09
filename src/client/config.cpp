#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static GatewayConfig g_gateway;
static char g_configPath[MAX_PATH];



/*
 * Initialize the configuration system.
 *
 * Finds config.ini next to the executable
 * and sets up the internal path buffer.
 */
void ConfigInitialize()
{
    char exePath[MAX_PATH];
    char *lastSlash;

    GetModuleFileNameA(
        NULL,
        exePath,
        sizeof(exePath)
    );

    lastSlash =
        strrchr(exePath, '\\');

    if (lastSlash != NULL)
    {
        *lastSlash = '\0';
    }

    wsprintfA(
        g_configPath,
        "%s\\config.ini",
        exePath
    );
}


/*
 * Load the gateway configuration from the INI file.
 *
 * The client stores only the gateway URL.
 * All provider-specific settings (model,
 * API key, system prompt, routing) belong
 * to the gateway, not to this client.
 */
void ConfigLoad()
{
    /*
     * Default gateway URL when no
     * configuration exists yet.
     */
    strncpy(
        g_gateway.url,
        "http://192.168.1.100:8000",
        sizeof(g_gateway.url) - 1
    );

    g_gateway.url[sizeof(g_gateway.url) - 1] = '\0';

    GetPrivateProfileStringA(
        "Gateway",
        "URL",
        "http://192.168.1.100:8000",
        g_gateway.url,
        sizeof(g_gateway.url),
        g_configPath
    );
}


/*
 * Save the gateway URL to the INI file.
 */
void ConfigSaveGateway()
{
    WritePrivateProfileStringA(
        "Gateway",
        "URL",
        g_gateway.url,
        g_configPath
    );
}


/*
 * Get the in-memory gateway configuration.
 */
GatewayConfig *ConfigGetGateway()
{
    return &g_gateway;
}


/*
 * Get configuration file path.
 */
const char *ConfigGetPath()
{
    return g_configPath;
}
