#include "http.h"

/*
 * WIN32_LEAN_AND_MEAN is already defined by http.h
 * before winsock2_compat.h is pulled in, so windows.h
 * will not pull in the conflicting winsock.h.
 */
#include <windows.h>

#include "config.h"
#include "gui.h"


/*
 * Application entry point.
 */
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    MSG msg;

    /*
     * Initialize the configuration system.
     */
    ConfigInitialize();
    ConfigLoad();

    /*
     * Initialize Winsock before the
     * gateway connection layer is used.
     */
    if (!HttpInitialize())
    {
        MessageBoxA(
            NULL,
            "Unable to initialize networking.",
            "Error",
            MB_ICONERROR
        );
        return 1;
    }

    /*
     * Create the main window.
     */
    if (GuiCreateMainWindow(
        hInstance,
        nCmdShow
    ) == NULL)
    {
        HttpShutdown();
        return 1;
    }


    /*
     * Main Windows message loop.
     */
    while (GetMessageA(
        &msg,
        NULL,
        0,
        0
    ))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    HttpShutdown();

    return msg.wParam;
}