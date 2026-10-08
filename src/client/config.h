#ifndef CLASSICAL_CONFIG_H
#define CLASSICAL_CONFIG_H

#include <windows.h>

#define MAX_PROVIDERS       16

#define MAX_PROVIDER_NAME   128
#define MAX_ENDPOINT        512
#define MAX_API_KEY         512
#define MAX_MODEL           256
#define MAX_SYSTEM_PROMPT   2048


/*
 * AI provider configuration.
 */
typedef struct
{
    char name[MAX_PROVIDER_NAME];
    char endpoint[MAX_ENDPOINT];
    char apiKey[MAX_API_KEY];
    char model[MAX_MODEL];
    char systemPrompt[MAX_SYSTEM_PROMPT];

} Provider;


/*
 * Configuration initialization.
 */
void ConfigInitialize();


/*
 * Load all providers from config.ini.
 */
void ConfigLoad();


/*
 * Save one provider.
 */
void ConfigSaveProvider(
    int index
);


/*
 * Save the active provider index.
 */
void ConfigSaveGeneral();


/*
 * Delete one provider.
 */
void ConfigDeleteProvider(
    int index
);


/*
 * Get provider count.
 */
int ConfigGetProviderCount();


/*
 * Get active provider index.
 */
int ConfigGetActiveProvider();


/*
 * Set active provider.
 */
void ConfigSetActiveProvider(
    int index
);


/*
 * Get provider by index.
 */
Provider *ConfigGetProvider(
    int index
);


/*
 * Create a default provider.
 */
void ConfigCreateDefaultProvider();


/*
 * Get configuration file path.
 */
const char *ConfigGetPath();

#endif