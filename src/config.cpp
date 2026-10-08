#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Provider g_providers[MAX_PROVIDERS];

static int g_providerCount = 0;
static int g_activeProvider = -1;

static char g_configPath[MAX_PATH];


/*
 * Clear a provider structure.
 */
static void ClearProvider(
    Provider *provider
)
{
    ZeroMemory(
        provider,
        sizeof(Provider)
    );

    strcpy(
        provider->model,
        "default"
    );

    strcpy(
        provider->systemPrompt,
        "You are a helpful AI assistant."
    );
}


/*
 * Generate an INI section name.
 */
static void GetProviderSection(
    int index,
    char *section,
    int size
)
{
    wsprintfA(
        section,
        "Provider%d",
        index
    );
}


/*
 * Load a provider from the INI file.
 */
static void LoadProviderFromIni(
    int index,
    Provider *provider
)
{
    char section[64];

    GetProviderSection(
        index,
        section,
        sizeof(section)
    );

    ClearProvider(provider);

    GetPrivateProfileStringA(
        section,
        "Name",
        "",
        provider->name,
        sizeof(provider->name),
        g_configPath
    );

    GetPrivateProfileStringA(
        section,
        "Endpoint",
        "",
        provider->endpoint,
        sizeof(provider->endpoint),
        g_configPath
    );

    GetPrivateProfileStringA(
        section,
        "ApiKey",
        "",
        provider->apiKey,
        sizeof(provider->apiKey),
        g_configPath
    );

    GetPrivateProfileStringA(
        section,
        "Model",
        "default",
        provider->model,
        sizeof(provider->model),
        g_configPath
    );

    GetPrivateProfileStringA(
        section,
        "SystemPrompt",
        "You are a helpful AI assistant.",
        provider->systemPrompt,
        sizeof(provider->systemPrompt),
        g_configPath
    );
}


/*
 * Save a provider to the INI file.
 */
void ConfigSaveProvider(
    int index
)
{
    char section[64];
    Provider *provider;

    if (index < 0 ||
        index >= g_providerCount)
    {
        return;
    }

    provider = &g_providers[index];

    GetProviderSection(
        index,
        section,
        sizeof(section)
    );

    WritePrivateProfileStringA(
        section,
        "Name",
        provider->name,
        g_configPath
    );

    WritePrivateProfileStringA(
        section,
        "Endpoint",
        provider->endpoint,
        g_configPath
    );

    WritePrivateProfileStringA(
        section,
        "ApiKey",
        provider->apiKey,
        g_configPath
    );

    WritePrivateProfileStringA(
        section,
        "Model",
        provider->model,
        g_configPath
    );

    WritePrivateProfileStringA(
        section,
        "SystemPrompt",
        provider->systemPrompt,
        g_configPath
    );
}


/*
 * Save the active provider index.
 */
void ConfigSaveGeneral()
{
    char buffer[32];

    wsprintfA(
        buffer,
        "%d",
        g_activeProvider
    );

    WritePrivateProfileStringA(
        "General",
        "ActiveProvider",
        buffer,
        g_configPath
    );
}


/*
 * Create a default provider.
 */
void ConfigCreateDefaultProvider()
{
    ClearProvider(
        &g_providers[0]
    );

    strcpy(
        g_providers[0].name,
        "Local Gateway"
    );

    strcpy(
        g_providers[0].endpoint,
        "http://192.168.1.100:8000/v1/chat"
    );

    g_providerCount = 1;
    g_activeProvider = 0;

    ConfigSaveProvider(0);
    ConfigSaveGeneral();
}


/*
 * Initialize configuration.
 */
void ConfigInitialize()
{
    char path[MAX_PATH];
    char *slash;

    GetModuleFileNameA(
        NULL,
        path,
        sizeof(path)
    );

    slash = strrchr(
        path,
        '\\'
    );

    if (slash != NULL)
    {
        *(slash + 1) = '\0';
    }

    wsprintfA(
        g_configPath,
        "%sconfig.ini",
        path
    );
}


/*
 * Load all providers.
 */
void ConfigLoad()
{
    int i;

    char section[64];
    char name[MAX_PROVIDER_NAME];
    char buffer[32];

    g_providerCount = 0;
    g_activeProvider = -1;

    for (i = 0;
         i < MAX_PROVIDERS;
         i++)
    {
        GetProviderSection(
            i,
            section,
            sizeof(section)
        );

        GetPrivateProfileStringA(
            section,
            "Name",
            "",
            name,
            sizeof(name),
            g_configPath
        );

        if (strlen(name) == 0)
        {
            break;
        }

        LoadProviderFromIni(
            i,
            &g_providers[i]
        );

        g_providerCount++;
    }

    /*
     * Create a default provider when
     * no configuration exists.
     */
    if (g_providerCount == 0)
    {
        ConfigCreateDefaultProvider();

        return;
    }

    GetPrivateProfileStringA(
        "General",
        "ActiveProvider",
        "0",
        buffer,
        sizeof(buffer),
        g_configPath
    );

    g_activeProvider = atoi(buffer);

    if (g_activeProvider < 0 ||
        g_activeProvider >= g_providerCount)
    {
        g_activeProvider = 0;
    }
}


/*
 * Delete a provider.
 */
void ConfigDeleteProvider(
    int index
)
{
    int i;
    char section[64];

    if (index < 0 ||
        index >= g_providerCount)
    {
        return;
    }

    if (g_providerCount <= 1)
    {
        return;
    }

    /*
     * Move following providers forward.
     */
    for (i = index;
         i < g_providerCount - 1;
         i++)
    {
        g_providers[i] =
            g_providers[i + 1];
    }

    g_providerCount--;

    /*
     * Remove the old last section.
     */
    GetProviderSection(
        g_providerCount,
        section,
        sizeof(section)
    );

    WritePrivateProfileStringA(
        section,
        NULL,
        NULL,
        g_configPath
    );

    /*
     * Rewrite the remaining providers.
     */
    for (i = 0;
         i < g_providerCount;
         i++)
    {
        ConfigSaveProvider(i);
    }

    if (g_activeProvider >= g_providerCount)
    {
        g_activeProvider =
            g_providerCount - 1;
    }

    ConfigSaveGeneral();
}


/*
 * Get provider count.
 */
int ConfigGetProviderCount()
{
    return g_providerCount;
}


/*
 * Get active provider.
 */
int ConfigGetActiveProvider()
{
    return g_activeProvider;
}


/*
 * Set active provider.
 */
void ConfigSetActiveProvider(
    int index
)
{
    if (index < 0 ||
        index >= g_providerCount)
    {
        return;
    }

    g_activeProvider = index;

    ConfigSaveGeneral();
}


/*
 * Get provider.
 */
Provider *ConfigGetProvider(
    int index
)
{
    if (index < 0 ||
        index >= g_providerCount)
    {
        return NULL;
    }

    return &g_providers[index];
}


/*
 * Get configuration path.
 */
const char *ConfigGetPath()
{
    return g_configPath;
}