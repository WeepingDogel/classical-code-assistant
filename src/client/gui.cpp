#include "gui.h"
#include "config.h"
#include "history.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>


/* -------------------------------------------------------
 * Compatibility definitions
 * ------------------------------------------------------- */

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


/* -------------------------------------------------------
 * Control IDs
 * ------------------------------------------------------- */

#define IDC_TABS            1000

#define IDC_PROVIDER_CHAT   1100
#define IDC_STATUS          1101
#define IDC_RESPONSE        1102
#define IDC_MESSAGE         1103
#define IDC_SEND            1104
#define IDC_PROGRESS        1105
#define IDC_HISTORY         1106

#define IDC_PROVIDER_LABEL  1200
#define IDC_PROVIDER        1201
#define IDC_ENDPOINT_LABEL  1202
#define IDC_ENDPOINT        1203
#define IDC_APIKEY_LABEL    1204
#define IDC_APIKEY          1205
#define IDC_MODEL_LABEL     1206
#define IDC_MODEL           1207
#define IDC_SYSTEM_LABEL    1208
#define IDC_SYSTEM          1209

#define IDC_NEW_PROVIDER    1210
#define IDC_SAVE_PROVIDER   1211
#define IDC_DELETE_PROVIDER 1212

#define TIMER_PROGRESS      1
#define TIMER_RESPONSE      2


/* -------------------------------------------------------
 * Globals
 * ------------------------------------------------------- */

static HINSTANCE g_hInstance;

static HWND hMain;
static HWND hTabs;


/* Chat */

static HWND hProviderChat;
static HWND hStatus;
static HWND hResponse;
static HWND hMessage;
static HWND hSend;
static HWND hProgress;
static HWND hHistory;

/*
 * Stashed copy of the last user message
 * so that FinishFakeRequest() can append
 * both sides of the exchange to the
 * history log.
 */
static char g_lastUserMessage[2048];


/* Settings */

static HWND hProvider;
static HWND hEndpoint;
static HWND hApiKey;
static HWND hModel;
static HWND hSystemPrompt;

static HWND hNewProvider;
static HWND hSaveProvider;
static HWND hDeleteProvider;

/* Settings labels */
static HWND hProviderLabel;
static HWND hEndpointLabel;
static HWND hApiKeyLabel;
static HWND hModelLabel;
static HWND hSystemPromptLabel;

static BOOL g_waiting = FALSE;
static int g_progressPosition = 0;


/* -------------------------------------------------------
 * Helper functions
 * ------------------------------------------------------- */

static HWND CreateLabel(
    HWND parent,
    const char *text,
    int id
)
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


/*
 * Update provider combo boxes.
 */
static void RefreshProviderCombos()
{
    int i;
    int active;

    active =
        ConfigGetActiveProvider();

    SendMessageA(
        hProviderChat,
        CB_RESETCONTENT,
        0,
        0
    );

    SendMessageA(
        hProvider,
        CB_RESETCONTENT,
        0,
        0
    );

    for (i = 0;
         i < ConfigGetProviderCount();
         i++)
    {
        Provider *provider;

        provider =
            ConfigGetProvider(i);

        SendMessageA(
            hProviderChat,
            CB_ADDSTRING,
            0,
            (LPARAM)provider->name
        );

        SendMessageA(
            hProvider,
            CB_ADDSTRING,
            0,
            (LPARAM)provider->name
        );
    }

    if (active >= 0)
    {
        SendMessageA(
            hProviderChat,
            CB_SETCURSEL,
            active,
            0
        );

        SendMessageA(
            hProvider,
            CB_SETCURSEL,
            active,
            0
        );
    }
}


/*
 * Load a provider into the settings page.
 */
static void LoadProviderToControls(
    int index
)
{
    Provider *provider;

    provider =
        ConfigGetProvider(index);

    if (provider == NULL)
    {
        return;
    }

    SetWindowTextA(
        hEndpoint,
        provider->endpoint
    );

    SetWindowTextA(
        hApiKey,
        provider->apiKey
    );

    SetWindowTextA(
        hModel,
        provider->model
    );

    SetWindowTextA(
        hSystemPrompt,
        provider->systemPrompt
    );
}


/*
 * Read settings controls into a provider.
 */
static void ReadControlsToProvider(
    int index
)
{
    Provider *provider;

    provider =
        ConfigGetProvider(index);

    if (provider == NULL)
    {
        return;
    }

    GetWindowTextA(
        hEndpoint,
        provider->endpoint,
        sizeof(provider->endpoint)
    );

    GetWindowTextA(
        hApiKey,
        provider->apiKey,
        sizeof(provider->apiKey)
    );

    GetWindowTextA(
        hModel,
        provider->model,
        sizeof(provider->model)
    );

    GetWindowTextA(
        hSystemPrompt,
        provider->systemPrompt,
        sizeof(provider->systemPrompt)
    );
}


/*
 * Select a provider.
 */
static void SelectProvider(
    int index
)
{
    if (index < 0 ||
        index >= ConfigGetProviderCount())
    {
        return;
    }

    /*
     * Save changes made to the previous provider.
     */
    if (ConfigGetActiveProvider() >= 0)
    {
        ReadControlsToProvider(
            ConfigGetActiveProvider()
        );

        ConfigSaveProvider(
            ConfigGetActiveProvider()
        );
    }

    ConfigSetActiveProvider(index);

    SendMessageA(
        hProviderChat,
        CB_SETCURSEL,
        index,
        0
    );

    SendMessageA(
        hProvider,
        CB_SETCURSEL,
        index,
        0
    );

    LoadProviderToControls(index);

    SetWindowTextA(
        hStatus,
        "Provider changed."
    );
}


/*
 * Add a new provider.
 */
static void AddProvider()
{
    int index;
    Provider *provider;

    index =
        ConfigGetProviderCount();

    if (index >= MAX_PROVIDERS)
    {
        MessageBoxA(
            hMain,
            "Maximum number of providers reached.",
            "Classical Code Assistant",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    /*
     * The configuration module currently keeps
     * providers internally. We use the last available
     * provider slot through ConfigGetProvider().
     *
     * This will be improved when the configuration API
     * gets an explicit ConfigAddProvider() function.
     */
    provider =
        ConfigGetProvider(index);

    /*
     * The current configuration API intentionally
     * keeps this version conservative.
     */
    MessageBoxA(
        hMain,
        "Provider creation will be enabled in the next configuration API revision.",
        "Classical Code Assistant",
        MB_OK | MB_ICONINFORMATION
    );
}


/*
 * Save the current provider.
 */
static void SaveProvider()
{
    int index;
    char name[128];

    index =
        ConfigGetActiveProvider();

    if (index < 0)
    {
        return;
    }

    GetWindowTextA(
        hProvider,
        name,
        sizeof(name)
    );

    if (strlen(name) == 0)
    {
        MessageBoxA(
            hMain,
            "Provider name cannot be empty.",
            "Classical Code Assistant",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    {
        Provider *provider;

        provider =
            ConfigGetProvider(index);

        if (provider != NULL)
        {
            strcpy(
                provider->name,
                name
            );
        }
    }

    ReadControlsToProvider(index);

    ConfigSaveProvider(index);
    ConfigSaveGeneral();

    RefreshProviderCombos();

    SetWindowTextA(
        hStatus,
        "Provider settings saved."
    );
}


/*
 * Delete current provider.
 */
static void DeleteProvider()
{
    int index;

    index =
        ConfigGetActiveProvider();

    if (ConfigGetProviderCount() <= 1)
    {
        MessageBoxA(
            hMain,
            "At least one provider must remain.",
            "Classical Code Assistant",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    if (MessageBoxA(
        hMain,
        "Delete the selected provider?",
        "Classical Code Assistant",
        MB_YESNO | MB_ICONQUESTION
    ) != IDYES)
    {
        return;
    }

    ConfigDeleteProvider(index);

    RefreshProviderCombos();

    LoadProviderToControls(
        ConfigGetActiveProvider()
    );

    SetWindowTextA(
        hStatus,
        "Provider deleted."
    );
}


/* -------------------------------------------------------
 * View chat history
 * ------------------------------------------------------- */

/*
 * Open the file picker in the chatlog
 * directory and load the chosen file
 * into the response box.
 */
static void ViewHistory()
{
    char logDir[MAX_PATH];
    static char buffer[262144];
    char fileName[MAX_PATH];
    char fileTitle[MAX_PATH];
    char status[300];
    OPENFILENAMEA ofn;

    /*
     * Make sure the log directory is
     * present so the picker can use
     * it as the starting location.
     */
    if (!HistoryEnsureDir())
    {
        MessageBoxA(
            hMain,
            "Unable to prepare the chat history directory.",
            "Classical Code Assistant",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    /*
     * GetOpenFileNameA needs the
     * directory as its initial folder.
     * Use HistoryFileFor(0) and trim
     * the file name, which gives us
     * the directory path cheaply.
     */
    {
        const char *path = HistoryFileFor(0);
        char *p;

        strncpy(
            logDir,
            path,
            sizeof(logDir) - 1
        );

        logDir[sizeof(logDir) - 1] = '\0';

        p = strrchr(logDir, '\\');

        if (p != NULL)
        {
            *(p + 1) = '\0';
        }
    }

    /*
     * If no log files exist yet,
     * inform the user and skip the
     * dialog.
     */
    if (!HistoryExists(ConfigGetActiveProvider()))
    {
        /*
         * Check whether ANY provider
         * has a log.  If none do,
         * say so.
         */
        int i;
        int any = 0;

        for (i = 0;
             i < ConfigGetProviderCount();
             i++)
        {
            if (HistoryExists(i))
            {
                any = 1;
                break;
            }
        }

        if (!any)
        {
            MessageBoxA(
                hMain,
                "No chat history yet.",
                "Classical Code Assistant",
                MB_OK | MB_ICONINFORMATION
            );

            return;
        }
    }

    ZeroMemory(
        &ofn,
        sizeof(ofn)
    );

    ofn.lStructSize =
        sizeof(ofn);

    ofn.hwndOwner =
        hMain;

    ofn.lpstrFilter =
        "Provider logs (*.txt)\0*.txt\0All files (*.*)\0*.*\0";

    ofn.nFilterIndex = 1;

    ofn.lpstrFile =
        fileName;

    ofn.nMaxFile =
        sizeof(fileName);

    ofn.lpstrFileTitle =
        fileTitle;

    ofn.nMaxFileTitle =
        sizeof(fileTitle);

    ofn.lpstrInitialDir =
        logDir;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_HIDEREADONLY |
        OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn) != TRUE)
    {
        /*
         * User cancelled.
         */
        return;
    }

    if (!HistoryLoad(
        fileName,
        buffer,
        sizeof(buffer)
    ))
    {
        MessageBoxA(
            hMain,
            "Unable to read the selected file.",
            "Classical Code Assistant",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    SetWindowTextA(
        hResponse,
        buffer
    );

    /*
     * Show which file is being
     * viewed in the status bar.
     */
    {
        const char *base;

        base = fileTitle;

        if (base == NULL ||
            base[0] == '\0')
        {
            base = ofn.lpstrFile;
        }

        wsprintfA(
            status,
            "Viewing history: %s",
            base
        );

        SetWindowTextA(
            hStatus,
            status
        );
    }
}


/* -------------------------------------------------------
 * Fake AI request
 * ------------------------------------------------------- */

static void StartFakeRequest()
{
    char message[2048];

    if (g_waiting)
    {
        return;
    }

    GetWindowTextA(
        hMessage,
        message,
        sizeof(message)
    );

    if (strlen(message) == 0)
    {
        MessageBoxA(
            hMain,
            "Please enter a message.",
            "Classical Code Assistant",
            MB_OK | MB_ICONINFORMATION
        );

        return;
    }

    /*
     * Remember the user text so it can
     * be logged together with the AI
     * reply when the request finishes.
     */
    strncpy(
        g_lastUserMessage,
        message,
        sizeof(g_lastUserMessage) - 1
    );

    g_lastUserMessage[
        sizeof(g_lastUserMessage) - 1
    ] = '\0';

    g_waiting = TRUE;
    g_progressPosition = 0;

    SetWindowTextA(
        hStatus,
        "Waiting for AI response..."
    );

    SetWindowTextA(
        hResponse,
        "Connecting to AI gateway..."
    );

    EnableWindow(
        hSend,
        FALSE
    );

    SetTimer(
        hMain,
        TIMER_PROGRESS,
        80,
        NULL
    );

    SetTimer(
        hMain,
        TIMER_RESPONSE,
        2000,
        NULL
    );
}


/*
 * Finish fake AI request.
 */
static void FinishFakeRequest()
{
    char response[4096];
    Provider *provider;

    provider =
        ConfigGetProvider(
            ConfigGetActiveProvider()
        );

    KillTimer(
        hMain,
        TIMER_PROGRESS
    );

    KillTimer(
        hMain,
        TIMER_RESPONSE
    );

    g_waiting = FALSE;

    SendMessageA(
        hProgress,
        PBM_SETPOS,
        100,
        0
    );

    SetWindowTextA(
        hStatus,
        "Ready"
    );

    if (provider != NULL)
    {
        wsprintfA(
            response,
            "Hello from Classical Code Assistant!\r\n"
            "\r\n"
            "This is a simulated response.\r\n"
            "\r\n"
            "Provider: %s\r\n"
            "Model: %s\r\n"
            "Endpoint: %s\r\n"
            "\r\n"
            "The HTTP gateway will be implemented next.",
            provider->name,
            provider->model,
            provider->endpoint
        );
    }
    else
    {
        strcpy(
            response,
            "No active provider."
        );
    }

    SetWindowTextA(
        hResponse,
        response
    );

    EnableWindow(
        hSend,
        TRUE
    );

    /*
     * Log this exchange to the active
     * provider's chat history file.
     * Failures are ignored: a history
     * write error should never block
     * the response from being shown.
     */
    HistoryAppend(
        ConfigGetActiveProvider(),
        g_lastUserMessage,
        response
    );
}


/* -------------------------------------------------------
 * Tab and layout
 * ------------------------------------------------------- */

static void GetTabPageRect(
    RECT *rect
)
{
    GetClientRect(
        hTabs,
        rect
    );

    SendMessageA(
        hTabs,
        TCM_ADJUSTRECT,
        FALSE,
        (LPARAM)rect
    );
}


static void UpdateTabVisibility()
{
    int tab;

    tab =
        (int)SendMessageA(
            hTabs,
            TCM_GETCURSEL,
            0,
            0
        );


    ShowWindow(
        hProviderChat,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hStatus,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hResponse,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hMessage,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hSend,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hProgress,
        tab == 0 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hHistory,
        tab == 0 ? SW_SHOW : SW_HIDE
    );


    ShowWindow(
        hProvider,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hEndpoint,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hApiKey,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hModel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hSystemPrompt,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hNewProvider,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hSaveProvider,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hDeleteProvider,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hProviderLabel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hEndpointLabel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hApiKeyLabel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hModelLabel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );

    ShowWindow(
        hSystemPromptLabel,
        tab == 1 ? SW_SHOW : SW_HIDE
    );
}


static void ResizeControls(
    HWND hwnd
)
{
    RECT client;
    RECT page;

    int x;
    int y;
    int width;
    int height;

    int labelWidth;
    int controlX;
    int controlWidth;

    GetClientRect(
        hwnd,
        &client
    );

    MoveWindow(
        hTabs,
        5,
        5,
        client.right - 10,
        client.bottom - 10,
        TRUE
    );

    GetTabPageRect(&page);

    x =
        page.left + 10;

    y =
        page.top + 10;

    width =
        page.right -
        page.left -
        20;

    height =
        page.bottom -
        page.top -
        20;


    /*
     * Chat page.
     */

    MoveWindow(
        hProviderChat,
        x,
        y,
        220,
        23,
        TRUE
    );

    MoveWindow(
        hStatus,
        x + 230,
        y + 3,
        width - 230,
        20,
        TRUE
    );

    MoveWindow(
        hResponse,
        x,
        y + 35,
        width,
        height - 125,
        TRUE
    );

    MoveWindow(
        hMessage,
        x,
        y + height - 75,
        width - 185,
        23,
        TRUE
    );

    MoveWindow(
        hHistory,
        x + width - 180,
        y + height - 75,
        100,
        23,
        TRUE
    );

    MoveWindow(
        hSend,
        x + width - 75,
        y + height - 75,
        75,
        23,
        TRUE
    );

    MoveWindow(
        hProgress,
        x,
        y + height - 40,
        width,
        18,
        TRUE
    );


    /*
     * Settings page.
     */

    labelWidth = 105;

    controlX =
        x + labelWidth;

    controlWidth =
        width - labelWidth;

    MoveWindow(
        hProviderLabel,
        x,
        y + 3,
        labelWidth - 5,
        20,
        TRUE
    );

    MoveWindow(
        hEndpointLabel,
        x,
        y + 38,
        labelWidth - 5,
        20,
        TRUE
    );

    MoveWindow(
        hApiKeyLabel,
        x,
        y + 73,
        labelWidth - 5,
        20,
        TRUE
    );

    MoveWindow(
        hModelLabel,
        x,
        y + 108,
        labelWidth - 5,
        20,
        TRUE
    );

    MoveWindow(
        hSystemPromptLabel,
        x,
        y + 143,
        labelWidth - 5,
        20,
        TRUE
    );


    MoveWindow(
        hProvider,
        controlX,
        y,
        controlWidth - 100,
        23,
        TRUE
    );

    MoveWindow(
        hNewProvider,
        controlX + controlWidth - 95,
        y,
        95,
        23,
        TRUE
    );

    MoveWindow(
        hEndpoint,
        controlX,
        y + 35,
        controlWidth,
        23,
        TRUE
    );

    MoveWindow(
        hApiKey,
        controlX,
        y + 70,
        controlWidth,
        23,
        TRUE
    );

    MoveWindow(
        hModel,
        controlX,
        y + 105,
        controlWidth,
        23,
        TRUE
    );

    MoveWindow(
        hSystemPrompt,
        controlX,
        y + 140,
        controlWidth,
        90,
        TRUE
    );

    MoveWindow(
        hSaveProvider,
        controlX,
        y + 245,
        100,
        25,
        TRUE
    );

    MoveWindow(
        hDeleteProvider,
        controlX + 110,
        y + 245,
        90,
        25,
        TRUE
    );
}


/* -------------------------------------------------------
 * Create controls
 * ------------------------------------------------------- */

static void CreateControls(
    HWND hwnd
)
{
    hTabs = CreateWindowA(
        "SysTabControl32",
        "",
        WS_CHILD |
        WS_VISIBLE |
        WS_CLIPSIBLINGS,
        5,
        5,
        500,
        400,
        hwnd,
        (HMENU)IDC_TABS,
        g_hInstance,
        NULL
    );


    /*
     * Local tab item structure.
     *
     * This avoids depending on TCITEMA,
     * which is missing from some old MinGW headers.
     */
    typedef struct
    {
        UINT mask;
        DWORD dwState;
        DWORD dwStateMask;
        LPSTR pszText;
        int cchTextMax;
        int iImage;
        LPARAM lParam;

    } CLASSICAL_TCITEMA;

    CLASSICAL_TCITEMA tabItem;

    ZeroMemory(
        &tabItem,
        sizeof(tabItem)
    );

    tabItem.mask = TCIF_TEXT;

    tabItem.pszText =
        (LPSTR)"Chat";

    SendMessageA(
        hTabs,
        TCM_INSERTITEMA,
        0,
        (LPARAM)&tabItem
    );

    tabItem.pszText =
        (LPSTR)"Settings";

    SendMessageA(
        hTabs,
        TCM_INSERTITEMA,
        1,
        (LPARAM)&tabItem
    );


    /*
     * Chat controls.
     */

    hProviderChat = CreateWindowA(
        "COMBOBOX",
        "",
        WS_CHILD |
        WS_VISIBLE |
        WS_TABSTOP |
        CBS_DROPDOWNLIST |
        WS_VSCROLL,
        0,
        0,
        220,
        200,
        hwnd,
        (HMENU)IDC_PROVIDER_CHAT,
        g_hInstance,
        NULL
    );


    hStatus =
        CreateLabel(
            hwnd,
            "Ready",
            IDC_STATUS
        );


    hResponse = CreateWindowA(
        "EDIT",
        "Classical Code Assistant is ready.",
        WS_CHILD |
        WS_VISIBLE |
        WS_BORDER |
        WS_VSCROLL |
        ES_MULTILINE |
        ES_AUTOVSCROLL |
        ES_READONLY,
        0,
        0,
        500,
        300,
        hwnd,
        (HMENU)IDC_RESPONSE,
        g_hInstance,
        NULL
    );


    hMessage = CreateWindowA(
        "EDIT",
        "",
        WS_CHILD |
        WS_VISIBLE |
        WS_BORDER |
        ES_AUTOHSCROLL,
        0,
        0,
        400,
        23,
        hwnd,
        (HMENU)IDC_MESSAGE,
        g_hInstance,
        NULL
    );


    hSend = CreateWindowA(
        "BUTTON",
        "Send",
        WS_CHILD |
        WS_VISIBLE |
        WS_TABSTOP |
        BS_PUSHBUTTON,
        0,
        0,
        75,
        23,
        hwnd,
        (HMENU)IDC_SEND,
        g_hInstance,
        NULL
    );


    hProgress = CreateWindowA(
        "msctls_progress32",
        "",
        WS_CHILD |
        WS_VISIBLE,
        0,
        0,
        500,
        18,
        hwnd,
        (HMENU)IDC_PROGRESS,
        g_hInstance,
        NULL
    );

    SendMessageA(
        hProgress,
        PBM_SETRANGE,
        0,
        MAKELPARAM(0, 100)
    );


    hHistory = CreateWindowA(
        "BUTTON",
        "View History",
        WS_CHILD |
        WS_VISIBLE |
        WS_TABSTOP |
        BS_PUSHBUTTON,
        0,
        0,
        100,
        23,
        hwnd,
        (HMENU)IDC_HISTORY,
        g_hInstance,
        NULL
    );


    /*
     * Settings controls.
     */

    hProviderLabel =
        CreateLabel(
            hwnd,
            "Provider:",
            IDC_PROVIDER_LABEL
        );

    hProvider = CreateWindowA(
        "COMBOBOX",
        "",
        WS_CHILD |
        WS_TABSTOP |
        WS_BORDER |
        CBS_DROPDOWN |
        WS_VSCROLL,
        0,
        0,
        300,
        200,
        hwnd,
        (HMENU)IDC_PROVIDER,
        g_hInstance,
        NULL
    );


    hNewProvider = CreateWindowA(
        "BUTTON",
        "Add Provider",
        WS_CHILD |
        WS_TABSTOP |
        BS_PUSHBUTTON,
        0,
        0,
        95,
        23,
        hwnd,
        (HMENU)IDC_NEW_PROVIDER,
        g_hInstance,
        NULL
    );


    hEndpointLabel =
        CreateLabel(
            hwnd,
            "Endpoint:",
            IDC_ENDPOINT_LABEL
        );

    hEndpoint = CreateWindowA(
        "EDIT",
        "",
        WS_CHILD |
        WS_BORDER |
        ES_AUTOHSCROLL,
        0,
        0,
        400,
        23,
        hwnd,
        (HMENU)IDC_ENDPOINT,
        g_hInstance,
        NULL
    );


    hApiKeyLabel =
        CreateLabel(
            hwnd,
            "API Key:",
            IDC_APIKEY_LABEL
        );

    hApiKey = CreateWindowA(
        "EDIT",
        "",
        WS_CHILD |
        WS_BORDER |
        ES_AUTOHSCROLL |
        ES_PASSWORD,
        0,
        0,
        400,
        23,
        hwnd,
        (HMENU)IDC_APIKEY,
        g_hInstance,
        NULL
    );


    hModelLabel =
        CreateLabel(
            hwnd,
            "Model:",
            IDC_MODEL_LABEL
        );

    hModel = CreateWindowA(
        "EDIT",
        "",
        WS_CHILD |
        WS_BORDER |
        ES_AUTOHSCROLL,
        0,
        0,
        400,
        23,
        hwnd,
        (HMENU)IDC_MODEL,
        g_hInstance,
        NULL
    );


    hSystemPromptLabel =
        CreateLabel(
            hwnd,
            "System Prompt:",
            IDC_SYSTEM_LABEL
        );

    hSystemPrompt = CreateWindowA(
        "EDIT",
        "",
        WS_CHILD |
        WS_BORDER |
        WS_VSCROLL |
        ES_MULTILINE |
        ES_AUTOVSCROLL,
        0,
        0,
        400,
        90,
        hwnd,
        (HMENU)IDC_SYSTEM,
        g_hInstance,
        NULL
    );


    hSaveProvider = CreateWindowA(
        "BUTTON",
        "Save Changes",
        WS_CHILD |
        WS_TABSTOP |
        BS_PUSHBUTTON,
        0,
        0,
        100,
        25,
        hwnd,
        (HMENU)IDC_SAVE_PROVIDER,
        g_hInstance,
        NULL
    );


    hDeleteProvider = CreateWindowA(
        "BUTTON",
        "Delete",
        WS_CHILD |
        WS_TABSTOP |
        BS_PUSHBUTTON,
        0,
        0,
        90,
        25,
        hwnd,
        (HMENU)IDC_DELETE_PROVIDER,
        g_hInstance,
        NULL
    );


    /*
     * Initialize provider selection.
     */
    RefreshProviderCombos();

    LoadProviderToControls(
        ConfigGetActiveProvider()
    );

    /*
     * Auto-load the active provider's
     * chat log into the response box,
     * if one exists.  This gives the
     * user a persistent view of the
     * conversation from the previous
     * run.
     */
    {
        int active;
        static char logBuffer[262144];
        char logPath[MAX_PATH];
        char loadStatus[300];
        const char *shortName;

        active =
            ConfigGetActiveProvider();

        if (active >= 0 &&
            HistoryExists(active))
        {
            /*
             * Copy the path out of the
             * static buffer returned by
             * HistoryFileFor so we
             * keep it stable for the
             * lifetime of this block.
             */
            strncpy(
                logPath,
                HistoryFileFor(active),
                sizeof(logPath) - 1
            );

            logPath[sizeof(logPath) - 1] = '\0';

            shortName =
                strrchr(logPath, '\\');

            if (shortName != NULL)
            {
                shortName++;
            }
            else
            {
                shortName = logPath;
            }

            if (HistoryLoad(
                logPath,
                logBuffer,
                sizeof(logBuffer)
            ))
            {
                SetWindowTextA(
                    hResponse,
                    logBuffer
                );

                wsprintfA(
                    loadStatus,
                    "Loaded history: %s",
                    shortName
                );

                SetWindowTextA(
                    hStatus,
                    loadStatus
                );
            }
        }
    }

    UpdateTabVisibility();
}


/* -------------------------------------------------------
 * Window procedure
 * ------------------------------------------------------- */

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
            CreateControls(hwnd);
            ResizeControls(hwnd);

            return 0;
        }


        case WM_SIZE:
        {
            ResizeControls(hwnd);

            return 0;
        }


        case WM_TIMER:
        {
            if (wParam == TIMER_PROGRESS)
            {
                g_progressPosition += 4;

                if (g_progressPosition > 100)
                {
                    g_progressPosition = 0;
                }

                SendMessageA(
                    hProgress,
                    PBM_SETPOS,
                    g_progressPosition,
                    0
                );

                return 0;
            }


            if (wParam == TIMER_RESPONSE)
            {
                FinishFakeRequest();

                return 0;
            }

            return 0;
        }


        case WM_COMMAND:
        {
            int id;
            int code;

            id =
                LOWORD(wParam);

            code =
                HIWORD(wParam);


            if (id == IDC_SEND)
            {
                StartFakeRequest();

                return 0;
            }


            if (id == IDC_NEW_PROVIDER)
            {
                AddProvider();

                return 0;
            }


            if (id == IDC_SAVE_PROVIDER)
            {
                SaveProvider();

                return 0;
            }


            if (id == IDC_DELETE_PROVIDER)
            {
                DeleteProvider();

                return 0;
            }


            if (id == IDC_HISTORY)
            {
                ViewHistory();

                return 0;
            }


            if (id == IDC_PROVIDER &&
                code == CBN_SELCHANGE)
            {
                int index;

                index =
                    (int)SendMessageA(
                        hProvider,
                        CB_GETCURSEL,
                        0,
                        0
                    );

                SelectProvider(index);

                return 0;
            }


            if (id == IDC_PROVIDER_CHAT &&
                code == CBN_SELCHANGE)
            {
                int index;

                index =
                    (int)SendMessageA(
                        hProviderChat,
                        CB_GETCURSEL,
                        0,
                        0
                    );

                SelectProvider(index);

                return 0;
            }

            return 0;
        }


        case WM_NOTIFY:
        {
            NMHDR *header;

            header =
                (NMHDR *)lParam;

            if (header->idFrom == IDC_TABS &&
                header->code == TCN_SELCHANGE)
            {
                UpdateTabVisibility();
                ResizeControls(hwnd);

                return 0;
            }

            return 0;
        }


        case WM_DESTROY:
        {
            KillTimer(
                hwnd,
                TIMER_PROGRESS
            );

            KillTimer(
                hwnd,
                TIMER_RESPONSE
            );

            PostQuitMessage(0);

            return 0;
        }
    }

    return DefWindowProcA(
        hwnd,
        msg,
        wParam,
        lParam
    );
}


/* -------------------------------------------------------
 * Main window creation
 * ------------------------------------------------------- */

HWND GuiCreateMainWindow(
    HINSTANCE hInstance,
    int nCmdShow
)
{
    WNDCLASSA wc;

    g_hInstance =
        hInstance;


    /*
     * Initialize common controls using
     * the old Win32 API.
     */
    InitCommonControls();


    ZeroMemory(
        &wc,
        sizeof(wc)
    );

    wc.style =
        CS_HREDRAW |
        CS_VREDRAW;

    wc.lpfnWndProc =
        GuiWindowProc;

    wc.hInstance =
        hInstance;

    wc.hIcon =
        LoadIconA(
            NULL,
            IDI_APPLICATION
        );

    wc.hCursor =
        LoadCursorA(
            NULL,
            IDC_ARROW
        );

    wc.hbrBackground =
        (HBRUSH)(
            COLOR_BTNFACE + 1
        );

    wc.lpszClassName =
        "ClassicalCodeAssistant";


    if (!RegisterClassA(&wc))
    {
        MessageBoxA(
            NULL,
            "Failed to register window class.",
            "Error",
            MB_OK | MB_ICONERROR
        );

        return NULL;
    }


    hMain = CreateWindowA(
        "ClassicalCodeAssistant",
        "Classical Code Assistant",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        760,
        560,
        NULL,
        NULL,
        hInstance,
        NULL
    );


    if (hMain == NULL)
    {
        MessageBoxA(
            NULL,
            "Failed to create main window.",
            "Error",
            MB_OK | MB_ICONERROR
        );

        return NULL;
    }


    ShowWindow(
        hMain,
        nCmdShow
    );

    UpdateWindow(hMain);

    return hMain;
}