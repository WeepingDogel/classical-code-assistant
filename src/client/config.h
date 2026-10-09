#ifndef CLASSICAL_CONFIG_H
#define CLASSICAL_CONFIG_H

#include <windows.h>

#define MAX_GATEWAY_URL 512


/*
 * Gateway connection configuration.
 *
 * The client knows only how to reach the
 * Linux AI gateway.  All provider details
 * (model, API key, system prompt, routing)
 * are handled on the server side.
 */
typedef struct
{
    char url[MAX_GATEWAY_URL];

} GatewayConfig;


/*
 * Configuration initialization.
 */
void ConfigInitialize();


/*
 * Load the gateway URL from config.ini.
 */
void ConfigLoad();


/*
 * Save the gateway URL to config.ini.
 */
void ConfigSaveGateway();


/*
 * Get the in-memory gateway configuration.
 */
GatewayConfig *ConfigGetGateway();


/*
 * Get configuration file path.
 */
const char *ConfigGetPath();

#endif