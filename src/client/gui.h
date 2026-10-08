#ifndef CLASSICAL_GUI_H
#define CLASSICAL_GUI_H

#include <windows.h>


/*
 * Create the main application window.
 */
HWND GuiCreateMainWindow(
    HINSTANCE hInstance,
    int nCmdShow
);


/*
 * Main window procedure.
 */
LRESULT CALLBACK GuiWindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
);

#endif