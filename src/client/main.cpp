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
     * Create the main window.
     */
    if (GuiCreateMainWindow(
        hInstance,
        nCmdShow
    ) == NULL)
    {
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

    return msg.wParam;
}