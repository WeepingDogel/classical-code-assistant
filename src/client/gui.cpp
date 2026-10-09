#include "gui.h"
#include "config.h"
#include "history.h"
#include "http.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>


/* Compatibility definitions for old MinGW */

#ifndef TCM_FIRST
#define TCM_FIRST 0x1300
#endif

#ifndef TCM_GETCURSEL
#define TCM_GETCURSEL (TCM_FIRST + 11)
#endif

#ifndef TCM_INSERTITEMA
#define TCM_INSERTITEMA (TCM_FIRST + 7)
#endif

#ifndef TCM_ADJUSTRECT
#define TCM_ADJUSTRECT (TCM_FIRST + 40)
#endif

#ifndef TCIF_TEXT
#define TCIF_TEXT 0x0001
#endif

#ifndef TCN_FIRST
#define TCN_FIRST ((UINT)-550)
#endif

#ifndef TCN_SELCHANGE
#define TCN_SELCHANGE (TCN_FIRST - 1)
#endif

#ifndef PBM_SETRANGE
#define PBM_SETRANGE (WM_USER + 1)
#endif

#ifndef PBM_SETPOS
#define PBM_SETPOS (WM_USER + 2)
#endif

#ifndef WC_TABCONTROL
#define WC_TABCONTROL "SysTabControl32"
#endif

#ifndef PROGRESSCLASS
#define PROGRESSCLASS "msctls_progress32"
#endif


/* Control IDs */

#define IDC_TABS            1000

#define IDC_STATUS          1101
#define IDC_RESPONSE        1102
#define IDC_MESSAGE         1103
#define IDC_SEND            1104
#define IDC_PROGRESS        1105
#define IDC_HISTORY         1106

#define IDC_GATEWAY_URL_LABEL 1200
#define IDC_GATEWAY_URL       1201
#define IDC_TEST_CONNECTION   1202
#define IDC_SAVE_SETTINGS     1203


/* Globals */

static HINSTANCE g_hInstance;
static HWND hMain;
static HWND hTabs;

/* Chat controls */
static HWND hStatus;
static HWND hResponse;
static HWND hMessage;
static HWND hSend;
static HWND hProgress;
static HWND hHistory;

/*
 * Stashed copy of the last user message
 * so the exchange can be logged to the
 * history file.
 */
static char g_lastUserMessage[2048];

/* Settings controls */
static HWND hGatewayUrlLabel;
static HWND hGatewayUrl;
static HWND hTestConnection;
static HWND hSaveSettings;

static BOOL g_waiting = FALSE;


/* Helper: create a static label */
static HWND CreateLabel(HWND parent, const char *text, int id)
{
    return CreateWindowA(
        "STATIC",
        text,
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        100,
        20,
        parent,
        (HMENU)id,
        g_hInstance,
        NULL
    );
}


/* Populate settings fields from config */
static void RefreshSettingsUI(void)
{
    GatewayConfig *gw = ConfigGetGateway();

    SetWindowTextA(
        hGatewayUrl,
        gw->url
    );
}


/* Copy settings fields into config */
static void ApplySettingsUI(void)
{
    GatewayConfig *gw = ConfigGetGateway();
    char url[MAX_GATEWAY_URL];

    GetWindowTextA(
        hGatewayUrl,
        url,
        sizeof(url)
    );

    if (url[0] != '\0')
    {
        strncpy(
            gw->url,
            url,
            sizeof(gw->url) - 1
        );

        gw->url[sizeof(gw->url) - 1] = '\0';
    }
}

/* Show or hide controls based on selected tab */
static void UpdateTabVisibility(void)
{
    int selectedTab;

    selectedTab = (int)SendMessageA(
        hTabs,
        TCM_GETCURSEL,
        0,
        0
    );

    if (selectedTab < 0)
    {
        selectedTab = 0;
    }

    if (selectedTab == 0)
    {
        if (hStatus)  ShowWindow(hStatus, SW_SHOW);
        if (hResponse) ShowWindow(hResponse, SW_SHOW);
        if (hMessage) ShowWindow(hMessage, SW_SHOW);
        if (hSend)    ShowWindow(hSend, SW_SHOW);
        if (hProgress) ShowWindow(hProgress, SW_SHOW);
        if (hHistory) ShowWindow(hHistory, SW_SHOW);

        if (hGatewayUrlLabel) ShowWindow(hGatewayUrlLabel, SW_HIDE);
        if (hGatewayUrl)      ShowWindow(hGatewayUrl, SW_HIDE);
        if (hTestConnection)  ShowWindow(hTestConnection, SW_HIDE);
        if (hSaveSettings)    ShowWindow(hSaveSettings, SW_HIDE);
    }
    else
    {
        if (hStatus)  ShowWindow(hStatus, SW_HIDE);
        if (hResponse) ShowWindow(hResponse, SW_HIDE);
        if (hMessage) ShowWindow(hMessage, SW_HIDE);
        if (hSend)    ShowWindow(hSend, SW_HIDE);
        if (hProgress) ShowWindow(hProgress, SW_HIDE);
        if (hHistory) ShowWindow(hHistory, SW_HIDE);

        if (hGatewayUrlLabel) ShowWindow(hGatewayUrlLabel, SW_SHOW);
        if (hGatewayUrl)      ShowWindow(hGatewayUrl, SW_SHOW);
        if (hTestConnection)  ShowWindow(hTestConnection, SW_SHOW);
        if (hSaveSettings)    ShowWindow(hSaveSettings, SW_SHOW);
    }
}


/* Resize child controls to fit the current client area */
static void ResizeControls(void)
{
    RECT rc;
    int tabH;
    int x;
    int y;
    int w;
    int h;

    GetClientRect(hMain, &rc);

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;

    tabH = 30;

    SendMessageA(
        hTabs,
        TCM_ADJUSTRECT,
        0,
        (LPARAM)&tabH
    );

    MoveWindow(
        hTabs,
        0,
        0,
        w,
        tabH,
        TRUE
    );

    x = 4;
    y = tabH + 2;
    w -= 8;

    if (w < 100)
    {
        w = 100;
    }

    /* Chat tab layout */
    {
        int contentH;
        int responseH;
        int messageH;

        contentH = h - tabH - 4;

        if (contentH < 100)
        {
            contentH = 100;
        }

        {
            int bottom = contentH - 28;

            responseH = (int)(bottom * 0.45);
            if (responseH < 40) responseH = 40;

            messageH = (int)(bottom * 0.25);
            if (messageH < 30) messageH = 30;

            MoveWindow(hStatus, x, y, w, 16, TRUE);
            y += 18;

            MoveWindow(hResponse, x, y, w, responseH, TRUE);
            y += responseH + 2;

            MoveWindow(hMessage, x, y, w, messageH, TRUE);
            y += messageH + 2;

            MoveWindow(hProgress, x, y, w - 160, 20, TRUE);
            MoveWindow(hSend, x + w - 130, y, 64, 23, TRUE);
            MoveWindow(hHistory, x + w - 62, y, 62, 23, TRUE);
        }
    }

    /* Settings tab layout */
    {
        int sy;

        sy = y + 2;

        MoveWindow(hGatewayUrlLabel, x, sy, w, 16, TRUE);
        sy += 20;

        MoveWindow(hGatewayUrl, x, sy, w, 22, TRUE);
        sy += 28;

        MoveWindow(hTestConnection, x, sy, 100, 23, TRUE);
        MoveWindow(hSaveSettings, x + 104, sy, 80, 23, TRUE);
    }
}


/* Set the status line text */
static void SetStatus(const char *text)
{
    SetWindowTextA(hStatus, text);
}


/*
 * Send the user message to the gateway,
 * display the reply, and log the exchange.
 */
static void SendChatMessage(void)
{
    char userMsg[2048];
    char response[HTTP_RESPONSE_SIZE];
    char statusMsg[256];
    GatewayConfig *gw;

    if (g_waiting)
    {
        return;
    }

    if (!GetWindowTextA(hMessage, userMsg, sizeof(userMsg)))
    {
        userMsg[0] = '\0';
    }

    if (userMsg[0] == '\0')
    {
        SetStatus("Enter a message first.");
        return;
    }

    /*
     * Stash the user message for
     * history logging.
     */
    strncpy(
        g_lastUserMessage,
        userMsg,
        sizeof(g_lastUserMessage) - 1
    );
    g_lastUserMessage[sizeof(g_lastUserMessage) - 1] = '\0';

    gw = ConfigGetGateway();

    if (gw->url[0] == '\0')
    {
        SetStatus("No gateway URL configured.");
        return;
    }

    g_waiting = TRUE;
    EnableWindow(hSend, FALSE);
    SendMessageA(
        hProgress,
        PBM_SETRANGE,
        0,
        0
    );
    SendMessageA(
        hProgress,
        PBM_SETPOS,
        0,
        0
    );
    SetStatus("Sending message to gateway...");

    if (HttpChatRequest(
        gw->url,
        userMsg,
        response,
        sizeof(response),
        statusMsg,
        sizeof(statusMsg)
    ))
    {
        SetStatus("Ready.");
        SetWindowTextA(hResponse, response);

        /*
         * Log the exchange to the
         * chat history file.
         */
        HistoryAppend(userMsg, response);

        /*
         * Clear the message box so the
         * user can type again.
         */
        SetWindowTextA(hMessage, "");
    }
    else
    {
        SetStatus(statusMsg);
    }

    g_waiting = FALSE;
    EnableWindow(hSend, TRUE);
}


/*
 * Test the gateway connection and
 * report the result in the status line.
 */
static void TestGatewayConnection(void)
{
    GatewayConfig *gw;
    char statusMsg[256];

    ApplySettingsUI();

    gw = ConfigGetGateway();

    if (gw->url[0] == '\0')
    {
        SetStatus("No gateway URL to test.");
        return;
    }

    SetStatus("Testing connection...");

    if (HttpTestConnection(
        gw->url,
        statusMsg,
        sizeof(statusMsg)
    ))
    {
        SetStatus("Gateway reachable.");
        MessageBoxA(
            hMain,
            "The gateway is reachable.",
            "Connection Test",
            MB_ICONINFORMATION
        );
    }
    else
    {
        SetStatus(statusMsg);
        MessageBoxA(
            hMain,
            statusMsg,
            "Connection Test",
            MB_ICONWARNING
        );
    }
}


/*
 * Save the gateway settings to the
 * INI configuration file.
 */
static void SaveGatewaySettings(void)
{
    ApplySettingsUI();

    ConfigSaveGateway();

    MessageBoxA(
        hMain,
        "Gateway settings saved.",
        "Settings",
        MB_ICONINFORMATION
    );
}


/*
 * Open the chat history file with the
 * system's default viewer using a
 * standard file-open dialog.
 */
static void ShowHistoryFile(void)
{
    const char *path;
    char buffer[4096];

    if (!HistoryExists())
    {
        MessageBoxA(
            hMain,
            "No chat history yet.",
            "History",
            MB_ICONINFORMATION
        );
        return;
    }

    path = HistoryFilePath();

    /*
     * Load the log into a buffer and
     * display it in a message box.
     * The open-file dialog is shown
     * first so the user can confirm
     * the path or pick a copy.
     */
    {
        OPENFILENAMEA ofn;

        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hMain;
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = buffer;
        ofn.nMaxFile = sizeof(buffer);
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHFILE;

        strncpy(
            buffer,
            path,
            sizeof(buffer) - 1
        );

        if (GetOpenFileNameA(&ofn))
        {
            char content[4096];

            if (!HistoryLoad(
                path,
                content,
                sizeof(content)
            ))
            {
                content[0] = '\0';
            }

            MessageBoxA(
                hMain,
                content,
                "Chat History",
                MB_OK
            );
        }
    }
}


/*
 * Main window procedure.
 *
 * Handles all messages for the main
 * application window and dispatches
 * command events to the action
 * handlers.
 */
LRESULT CALLBACK GuiWindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            int i;
            LRESULT ret;

            /*
             * Tab control.
             */
            hTabs = CreateWindowA(
                WC_TABCONTROL,
                "",
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                0,
                0,
                300,
                30,
                hwnd,
                (HMENU)IDC_TABS,
                g_hInstance,
                NULL
            );

            if (hTabs == NULL)
            {
                return -1;
            }

            for (i = 0; i < 2; i++)
            {
                char title[32];

                if (i == 0)
                {
                    strcpy(title, "Chat");
                }
                else
                {
                    strcpy(title, "Settings");
                }

                ret = SendMessageA(
                    hTabs,
                    TCM_INSERTITEMA,
                    i,
                    (LPARAM)title
                );

                if (ret == 0)
                {
                    return -1;
                }
            }

            /*
             * Chat controls.
             */
            hStatus = CreateLabel(
                hwnd,
                "Ready.",
                IDC_STATUS
            );

            hResponse = CreateWindowA(
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE |
                WS_BORDER | WS_VSCROLL |
                ES_MULTILINE | ES_READONLY |
                ES_AUTOVSCROLL | ES_WANTRETURN,
                0,
                0,
                300,
                200,
                hwnd,
                (HMENU)IDC_RESPONSE,
                g_hInstance,
                NULL
            );

            hMessage = CreateWindowA(
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE |
                WS_BORDER | WS_VSCROLL |
                ES_MULTILINE | ES_AUTOVSCROLL |
                ES_WANTRETURN,
                0,
                0,
                300,
                80,
                hwnd,
                (HMENU)IDC_MESSAGE,
                g_hInstance,
                NULL
            );

            hProgress = CreateWindowA(
                PROGRESSCLASS,
                "",
                WS_CHILD | WS_VISIBLE,
                0,
                0,
                300,
                20,
                hwnd,
                (HMENU)IDC_PROGRESS,
                g_hInstance,
                NULL
            );

            hSend = CreateWindowA(
                "BUTTON",
                "Send",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,
                0,
                64,
                23,
                hwnd,
                (HMENU)IDC_SEND,
                g_hInstance,
                NULL
            );

            hHistory = CreateWindowA(
                "BUTTON",
                "History",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,
                0,
                62,
                23,
                hwnd,
                (HMENU)IDC_HISTORY,
                g_hInstance,
                NULL
            );

            /*
             * Settings controls.
             */
            hGatewayUrlLabel = CreateLabel(
                hwnd,
                "Gateway URL:",
                IDC_GATEWAY_URL_LABEL
            );

            hGatewayUrl = CreateWindowA(
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE |
                WS_BORDER | ES_AUTOHSCROLL,
                0,
                0,
                300,
                22,
                hwnd,
                (HMENU)IDC_GATEWAY_URL,
                g_hInstance,
                NULL
            );

            hTestConnection = CreateWindowA(
                "BUTTON",
                "Test",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,
                0,
                100,
                23,
                hwnd,
                (HMENU)IDC_TEST_CONNECTION,
                g_hInstance,
                NULL
            );

            hSaveSettings = CreateWindowA(
                "BUTTON",
                "Save",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,
                0,
                80,
                23,
                hwnd,
                (HMENU)IDC_SAVE_SETTINGS,
                g_hInstance,
                NULL
            );

            ResizeControls();
            UpdateTabVisibility();
            RefreshSettingsUI();

            break;
        }

        case WM_SIZE:
        {
            ResizeControls();
            break;
        }

        case WM_COMMAND:
        {
            int controlId;
            int code;

            controlId = LOWORD(wParam);
            code = HIWORD(wParam);

            switch (controlId)
            {
                case IDC_SEND:
                case IDC_HISTORY:
                case IDC_TEST_CONNECTION:
                case IDC_SAVE_SETTINGS:

                    if (code == BN_CLICKED)
                    {
                        if (controlId == IDC_SEND)
                        {
                            SendChatMessage();
                        }
                        else if (controlId == IDC_HISTORY)
                        {
                            ShowHistoryFile();
                        }
                        else if (controlId == IDC_TEST_CONNECTION)
                        {
                            TestGatewayConnection();
                        }
                        else if (controlId == IDC_SAVE_SETTINGS)
                        {
                            SaveGatewaySettings();
                        }
                    }
                    break;

                default:
                    break;
            }
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh;

            pnmh = (LPNMHDR)lParam;

            if (pnmh != NULL &&
                pnmh->idFrom == IDC_TABS &&
                pnmh->code == TCN_SELCHANGE)
            {
                UpdateTabVisibility();
                ResizeControls();
            }
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            break;
    }

    return DefWindowProcA(
        hwnd,
        msg,
        wParam,
        lParam
    );
}


/*
 * Create the main application window.
 *
 * Registers the window class, creates
 * the top-level window, and shows it.
 * Returns the HWND on success, NULL
 * on failure.
 */
HWND GuiCreateMainWindow(
    HINSTANCE hInstance,
    int nCmdShow
)
{
    WNDCLASA wc;

    g_hInstance = hInstance;

    memset(&wc, 0, sizeof(wc));

    wc.style = 0;
    wc.lpfnWndProc = GuiWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(
        hInstance,
        NULL
    );
    wc.hCursor = LoadCursorA(
        NULL,
        (LPCSTR)IDC_ARROW
    );
    wc.hbrBackground = (HBRUSH)GetStockObject(
        COLOR_BTNFACE
    );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ClassicalAssistant";

    if (RegisterClassA(&wc) == 0)
    {
        /*
         * The class may already be
         * registered.  Only fail
         * on a real error.
         */
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return NULL;
        }
    }

    hMain = CreateWindowExA(
        0,
        "ClassicalAssistant",
        "Classical Code Assistant",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        700,
        500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hMain == NULL)
    {
        return NULL;
    }

    ShowWindow(hMain, nCmdShow);
    UpdateWindow(hMain);

    return hMain;
}