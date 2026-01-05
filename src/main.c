#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#include "ini.h"
#include <stdbool.h>
#include "bandwidth.h"
#include "drop.h"
#include "duplicate.h"
#include "lag.h"
#include "ood.h"
#include "reset.h"
#include "tamper.h"
#include "throttle.h"
#include "disconnect.h"
#include <process.h> 

int running = 0;
int Mode = 0;
char timerbuffer[6];
int currentPresetIndex = 0; // Track current preset (0-4)

// Helper macro and functions
#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

static inline bool parseBool(const char* value) {
    return strcmp(value, "true") == 0;
}

void preset1_config(void);
void preset2_config(void);
void preset3_config(void);
void preset4_config(void);
void preset5_config(void);
void intToStr(int num, char* str);



typedef struct {
   const char* Keybind;
} General;

typedef struct {
    const char* PresetName;
    bool Lag_Inbound;
    bool Lag_Outbound;
    const char* Lag_Delay;
    bool Drop_Inbound;
    bool Drop_Outbound;
    const char* Drop_Chance;
    bool Disconnect_Inbound;
    bool Disconnect_Outbound;
    const char* BandwidthLimiter_QueueSize;
    const char* BandwidthLimiter_Size;
    bool BandwidthLimiter_Inbound;
    bool BandwidthLimiter_Outbound;
    const char* BandwidthLimiter_Limit;
    bool Throttle_DropThrottled;
    const char* Throttle_Timeframe;
    bool Throttle_Inbound;
    bool Throttle_Outbound;
    const char* Throttle_Chance;
    const char* Duplicate_Count;
    bool Duplicate_Inbound;
    bool Duplicate_Outbound;
    const char* Duplicate_Chance;
    bool OutOfOrder_Inbound;
    bool OutOfOrder_Outbound;
    const char* OutOfOrder_Chance;
    bool Tamper_RedoChecksum;
    bool Tamper_Inbound;
    bool Tamper_Outbound;
    const char* Tamper_Chance;
    bool SetTCPRST_Inbound;
    bool SetTCPRST_Outbound;
    const char* SetTCPRST_Chance;
} PresetConfig;

typedef PresetConfig Preset1;
typedef PresetConfig Preset2;
typedef PresetConfig Preset3;
typedef PresetConfig Preset4;
typedef PresetConfig Preset5;

struct General {
    const char* Keybind;
};

General general = {
    "["
};

// Initialize Preset1 structure with default values
Preset1 preset1 = { .PresetName = "Preset 1"};

// Initialize Preset1 structure with default values
Preset2 preset2 = { .PresetName = "Preset 2"};

// Initialize Preset1 structure with default values
Preset3 preset3 = { .PresetName = "Preset 3"};

// Initialize Preset1 structure with default values
Preset4 preset4 = { .PresetName = "Preset 4"};

// Initialize Preset1 structure with default values
Preset5 preset5 = { .PresetName = "Preset 5"};

// Lookup tables for preset configuration
static const PresetConfig* presetLookup[] = {&preset1, &preset2, &preset3, &preset4, &preset5};
static void (*presetConfigFuncs[])(void) = {preset1_config, preset2_config, preset3_config, preset4_config, preset5_config};



static int handler3(void* user, const char* section, const char* name, const char* value) {
    General* generalconfig = (General*)user;

    if (MATCH("General", "Keybind")) {
        // Use _strdup if your environment requires it
        generalconfig->Keybind = _strdup(value);

        // Optionally, handle memory allocation errors
        if (generalconfig->Keybind == NULL) {
            return -1; // Indicate an error
        }
    }

    return 0; // Indicate success
}

// Generic preset handler to eliminate code duplication
static int handlerPreset(void* user, const char* section, const char* name,
                        const char* value, const char* expectedSection) {
    PresetConfig* pconfig = (PresetConfig*)user;

    if (!MATCH(expectedSection, "")) {
        if (strcmp(section, expectedSection) != 0) {
            return 0;
        }
    }

    if (strcmp(name, "PresetName") == 0) {
        pconfig->PresetName = _strdup(value);
    }
    else if (strcmp(name, "Lag_Inbound") == 0) {
        pconfig->Lag_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Lag_Outbound") == 0) {
        pconfig->Lag_Outbound = parseBool(value);
    }
    else if (strcmp(name, "Lag_Delay") == 0) {
        pconfig->Lag_Delay = _strdup(value);
    }
    else if (strcmp(name, "Drop_Inbound") == 0) {
        pconfig->Drop_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Drop_Outbound") == 0) {
        pconfig->Drop_Outbound = parseBool(value);
    }
    else if (strcmp(name, "Drop_Chance") == 0) {
        pconfig->Drop_Chance = _strdup(value);
    }
    else if (strcmp(name, "Disconnect_Inbound") == 0) {
        pconfig->Disconnect_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Disconnect_Outbound") == 0) {
        pconfig->Disconnect_Outbound = parseBool(value);
    }
    else if (strcmp(name, "BandwidthLimiter_QueueSize") == 0) {
        pconfig->BandwidthLimiter_QueueSize = _strdup(value);
    }
    else if (strcmp(name, "BandwidthLimiter_Size") == 0) {
        pconfig->BandwidthLimiter_Size = _strdup(value);
    }
    else if (strcmp(name, "BandwidthLimiter_Inbound") == 0) {
        pconfig->BandwidthLimiter_Inbound = parseBool(value);
    }
    else if (strcmp(name, "BandwidthLimiter_Outbound") == 0) {
        pconfig->BandwidthLimiter_Outbound = parseBool(value);
    }
    else if (strcmp(name, "BandwidthLimiter_Limit") == 0) {
        pconfig->BandwidthLimiter_Limit = _strdup(value);
    }
    else if (strcmp(name, "Throttle_DropThrottled") == 0) {
        pconfig->Throttle_DropThrottled = parseBool(value);
    }
    else if (strcmp(name, "Throttle_Timeframe") == 0) {
        pconfig->Throttle_Timeframe = _strdup(value);
    }
    else if (strcmp(name, "Throttle_Inbound") == 0) {
        pconfig->Throttle_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Throttle_Outbound") == 0) {
        pconfig->Throttle_Outbound = parseBool(value);
    }
    else if (strcmp(name, "Throttle_Chance") == 0) {
        pconfig->Throttle_Chance = _strdup(value);
    }
    else if (strcmp(name, "Duplicate_Count") == 0) {
        pconfig->Duplicate_Count = _strdup(value);
    }
    else if (strcmp(name, "Duplicate_Inbound") == 0) {
        pconfig->Duplicate_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Duplicate_Outbound") == 0) {
        pconfig->Duplicate_Outbound = parseBool(value);
    }
    else if (strcmp(name, "Duplicate_Chance") == 0) {
        pconfig->Duplicate_Chance = _strdup(value);
    }
    else if (strcmp(name, "OutOfOrder_Inbound") == 0) {
        pconfig->OutOfOrder_Inbound = parseBool(value);
    }
    else if (strcmp(name, "OutOfOrder_Outbound") == 0) {
        pconfig->OutOfOrder_Outbound = parseBool(value);
    }
    else if (strcmp(name, "OutOfOrder_Chance") == 0) {
        pconfig->OutOfOrder_Chance = _strdup(value);
    }
    else if (strcmp(name, "Tamper_RedoChecksum") == 0) {
        pconfig->Tamper_RedoChecksum = parseBool(value);
    }
    else if (strcmp(name, "Tamper_Inbound") == 0) {
        pconfig->Tamper_Inbound = parseBool(value);
    }
    else if (strcmp(name, "Tamper_Outbound") == 0) {
        pconfig->Tamper_Outbound = parseBool(value);
    }
    else if (strcmp(name, "Tamper_Chance") == 0) {
        pconfig->Tamper_Chance = _strdup(value);
    }
    else if (strcmp(name, "SetTCPRST_Inbound") == 0) {
        pconfig->SetTCPRST_Inbound = parseBool(value);
    }
    else if (strcmp(name, "SetTCPRST_Outbound") == 0) {
        pconfig->SetTCPRST_Outbound = parseBool(value);
    }
    else if (strcmp(name, "SetTCPRST_Chance") == 0) {
        pconfig->SetTCPRST_Chance = _strdup(value);
    }
    else {
        return 0;  // unknown section/name, error
    }

    return 1;  // Success
}

static int handler1(void* user, const char* section, const char* name, const char* value) {
    return handlerPreset(user, section, name, value, "Preset1");
}
static int handler2(void* user, const char* section, const char* name, const char* value) {
    return handlerPreset(user, section, name, value, "Preset2");
}
static int handler4(void* user, const char* section, const char* name, const char* value) {
    return handlerPreset(user, section, name, value, "Preset3");
}
static int handler5(void* user, const char* section, const char* name, const char* value) {
    return handlerPreset(user, section, name, value, "Preset4");
}
static int handler6(void* user, const char* section, const char* name, const char* value) {
    return handlerPreset(user, section, name, value, "Preset5");
}

// ! the order decides which module get processed first
Module* modules[MODULE_CNT] = {
    &lagModule,
    &dropModule,
    &disconnectModule,
    &bandwidthModule,
    &throttleModule,
    &dupModule,
    &oodModule,
    &tamperModule,
    &resetModule,
};

volatile short sendState = SEND_STATUS_NONE;

// global iup handlers
static Ihandle *dialog, *topFrame, * middleFrame, * lowerMiddleFrame, * bottomFrame;
static Ihandle *statusLabel;
static Ihandle* filterText, * filterButton;
static Ihandle* label1, * label2;

// Function prototype for timer callback
static int timerDelayCb(Ihandle* ih);


Ihandle* filterSelectList;
Ihandle* filterSelectList2;
Ihandle* filterSelectList3;
Ihandle* filterSelectList4;
Ihandle* filterSelectList5;

int DelayTimerValue=1000;


Ihandle* TimerLabel;

// timer to update icons
static Ihandle *stateIcon;
static Ihandle *timer;
static Ihandle *timeout = NULL;

void showStatus(const char *line);
static int uiOnDialogShow(Ihandle *ih, int state);
static int uiStopCb(Ihandle *ih);
static int uiStartCb(Ihandle *ih);
static int uiTimerCb(Ihandle *ih);
static int uiTimeoutCb(Ihandle *ih);
static int uiListSelectCb(Ihandle* ih, char* text, int item, int state);
static int uiList2SelectCb(Ihandle* ih, char* text, int item, int state);
static int uiList3SelectCb(Ihandle* ih, char* text, int item, int state);
static int uiList4SelectCb(Ihandle* ih, char* text, int item, int state);
static int uiList5SelectCb(Ihandle* ih, char* text, int item, int state);






static int uiFilterTextCb(Ihandle *ih);
static void uiSetupModule(Module *module, Ihandle *parent);

// serializing config files using a stupid custom format
#define CONFIG_FILE "config.txt"
#define CONFIG_MAX_RECORDS 64
#define CONFIG_BUF_SIZE 4096
typedef struct {
    char* filterName;
    char* filterValue;
} filterRecord;
UINT filtersSize;
filterRecord filters[CONFIG_MAX_RECORDS] = {0};
char configBuf[CONFIG_BUF_SIZE+2]; // add some padding to write \n
BOOL parameterized = 0; // parameterized flag, means reading args from command line

// loading up filters and fill in
void loadConfig() {
    char path[MSG_BUFSIZE];
    char *p;
    FILE *f;
    GetModuleFileName(NULL, path, MSG_BUFSIZE);
    LOG("Executable path: %s", path);
    p = strrchr(path, '\\');
    if (p == NULL) p = strrchr(path, '/'); // holy shit
    strcpy(p+1, CONFIG_FILE);
    LOG("Config path: %s", path);
    f = fopen(path, "r");
    if (f) {
        size_t len;
        char *current, *last;
        len = fread(configBuf, sizeof(char), CONFIG_BUF_SIZE, f);
        if (len == CONFIG_BUF_SIZE) {
            LOG("Config file is larger than %d bytes, get truncated.", CONFIG_BUF_SIZE);
        }
        // always patch in a newline at the end to ease parsing
        configBuf[len] = '\n';
        configBuf[len+1] = '\0';

        // parse out the kv pairs. isn't quite safe
        filtersSize = 0;
        last = current = configBuf;
        do {
            // eat up empty lines
EAT_SPACE:  while (isspace(*current)) { ++current; }
            if (*current == '#') {
                current = strchr(current, '\n');
                if (!current) break;
                current = current + 1;
                goto EAT_SPACE;
            }

            // now we can start
            last = current;
            current = strchr(last, ':');
            if (!current) break;
            *current = '\0';
            filters[filtersSize].filterName = last;
            current += 1;
            while (isspace(*current)) { ++current; } // eat potential space after :
            last = current;
            current = strchr(last, '\n');
            if (!current) break;
            filters[filtersSize].filterValue = last;
            *current = '\0';
            if (*(current-1) == '\r') *(current-1) = 0;
            last = current = current + 1;
            ++filtersSize;
        } while (last && last - configBuf < CONFIG_BUF_SIZE);
        LOG("Loaded %u records.", filtersSize);
    }

    if (!f || filtersSize == 0)
    {
        LOG("Failed to load from config. Fill in a simple one.");
        // config is missing or ill-formed. fill in some simple ones
        filters[filtersSize].filterName = "loopback packets";
        filters[filtersSize].filterValue = "outbound and ip.DstAddr >= 127.0.0.1 and ip.DstAddr <= 127.255.255.255";
        filtersSize = 1;
    }
}

void init(int argc, char* argv[]) {
    UINT ix;
    Ihandle* topVbox, * bottomVbox, * dialogVBox, * controlHbox;
    Ihandle* noneIcon, * doingIcon, * errorIcon;
    char* arg_value = NULL;

    // fill in config
    loadConfig();

    // iup inits
    IupOpen(&argc, &argv);

    // this is so easy to get wrong so it's pretty worth noting in the program
    statusLabel = IupLabel("NOTICE: When capturing localhost (loopback) packets, you CAN'T include inbound criteria.\n"
        "Filters like 'udp' need to be 'udp and outbound' to work. See readme for more info.");
    IupSetAttribute(statusLabel, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusLabel, "PADDING", "8x8");

    topFrame = IupFrame(
        topVbox = IupVbox(
            filterText = IupText(NULL),
            controlHbox = IupHbox(
                stateIcon = IupLabel(NULL),
                filterButton = IupButton("Start", NULL),
                IupLabel("Network:  "),
                filterSelectList2 = IupList(NULL),
                IupFill(),
                IupLabel("Presets:  "),
                filterSelectList = IupList(NULL),
                NULL
            ),
            NULL
        )
    );

      controlHbox = IupHbox(
          IupLabel("Function Presets:  "),
          filterSelectList3 = IupList(NULL),
          IupFill(),

          IupLabel("Trigger Mode"),
          filterSelectList4 = IupList(NULL),

          IupFill(),

          TimerLabel = IupLabel("Timer:"),
          filterSelectList5 = IupList(NULL),
        NULL
    );

      //filterSelectList5
    lowerMiddleFrame = IupFrame(
        controlHbox
    );
    IupHide(filterSelectList5);
    IupHide(TimerLabel);

    // Define the keybind as a constant character string
    const char* keybind = general.Keybind;

    // Base text with placeholders for the keybind variable
    const char* baseText = "Use the key %s to toggle on/off\nThis is a modified version of clumsy to use hotkeys to toggle on/off coded by mmaxx45\nUse Ctrl + %s for next preset, Shift + %s for previous preset";

    // Calculate the length needed for the formatted string (3 keybind placeholders)
    int len = snprintf(NULL, 0, baseText, keybind, keybind, keybind) + 1;

    // Allocate memory for the label text
    char* labelText = (char*)malloc(len);
    if (labelText == NULL) {
        perror("Failed to allocate memory");
        // Handle the error appropriately (e.g., log it, clean up resources)
        return;  // Simply return without a value
    }

    // Construct the label text with the keybind variable
    snprintf(labelText, len, baseText, keybind, keybind, keybind);
    middleFrame = IupFrame(
        label1 = IupLabel(labelText)
    );


    // parse arguments and set globals *before* setting up UI.
    // arguments can be read and set after callbacks are setup
    // FIXME as Release is built as WindowedApp, stdout/stderr won't show
    LOG("argc: %d", argc);
    if (argc > 1) {
        if (!parseArgs(argc, argv)) {
            fprintf(stderr, "invalid argument count. ensure you're using options as \"--drop on\"");
            exit(-1); // fail fast.
        }
        parameterized = 1;
    }
    IupSetAttribute(label1, "ALIGNMENT", "ACENTER");
    IupSetAttribute(topFrame, "TITLE", "Filtering");
    IupSetAttribute(topFrame, "EXPAND", "HORIZONTAL");
    IupSetAttribute(filterText, "EXPAND", "HORIZONTAL");
    IupSetCallback(filterText, "VALUECHANGED_CB", (Icallback)uiFilterTextCb);
    IupSetAttribute(filterButton, "PADDING", "8x");
    IupSetCallback(filterButton, "ACTION", uiStartCb);
    IupSetAttribute(topVbox, "NCMARGIN", "4x4");
    IupSetAttribute(topVbox, "NCGAP", "4x2");
    IupSetAttribute(controlHbox, "ALIGNMENT", "ACENTER");

    // setup state icon
    IupSetAttribute(stateIcon, "IMAGE", "none_icon");
    IupSetAttribute(stateIcon, "PADDING", "4x");

    // fill in options and setup callback
    IupSetAttribute(filterSelectList, "VISIBLECOLUMNS", "24");
    IupSetAttribute(filterSelectList, "DROPDOWN", "YES");

    IupSetAttribute(filterSelectList2, "VISIBLECOLUMNS", "15");
    IupSetAttribute(filterSelectList2, "DROPDOWN", "YES");
    IupSetAttributes(filterSelectList2, "1=(Local)_This_Device, 2=(Remote)_Shared_Devices");
    IupSetAttribute(filterSelectList2, "VALUE", "2");
    IupSetCallback(filterSelectList2, "ACTION", (Icallback)uiList2SelectCb);

    for (ix = 0; ix < filtersSize; ++ix) {
        char ixBuf[4];
        sprintf(ixBuf, "%d", ix+1); // ! staring from 1, following lua indexing
        IupStoreAttribute(filterSelectList, ixBuf, filters[ix].filterName);
    }
    IupSetAttribute(filterSelectList, "VALUE", "1");

    IupSetAttribute(filterSelectList3, "VALUE", "1");
    IupSetAttribute(filterSelectList4, "VALUE", "1");

    IupSetCallback(filterSelectList, "ACTION", (Icallback)uiListSelectCb);
    // set filter text value since the callback won't take effect before main loop starts
    IupSetAttribute(filterText, "VALUE", filters[0].filterValue);

    // functionalities frame
    bottomFrame = IupFrame(
        bottomVbox = IupVbox(
            NULL
        )
    );
    IupSetAttribute(bottomFrame, "TITLE", "Functions");
    IupSetAttribute(bottomVbox, "NCMARGIN", "4x4");
    IupSetAttribute(bottomVbox, "NCGAP", "4x2");

    // create icons
    noneIcon = IupImage(8, 8, icon8x8);
    doingIcon = IupImage(8, 8, icon8x8);
    errorIcon = IupImage(8, 8, icon8x8);
    IupSetAttribute(noneIcon, "0", "BGCOLOR");
    IupSetAttribute(noneIcon, "1", "224 224 224");
    IupSetAttribute(doingIcon, "0", "BGCOLOR");
    IupSetAttribute(doingIcon, "1", "109 170 44");
    IupSetAttribute(errorIcon, "0", "BGCOLOR");
    IupSetAttribute(errorIcon, "1", "208 70 72");
    IupSetHandle("none_icon", noneIcon);
    IupSetHandle("doing_icon", doingIcon);
    IupSetHandle("error_icon", errorIcon);



    IupSetAttribute(label1, "EXPAND", "HORIZONTAL");
    IupSetAttribute(middleFrame, "TITLE", "Info");
    IupSetAttribute(middleFrame, "EXPAND", "HORIZONTAL");

    IupSetAttribute(label2, "EXPAND", "HORIZONTAL");
    IupSetAttribute(lowerMiddleFrame, "TITLE", "Extra Presets");
    IupSetAttribute(lowerMiddleFrame, "EXPAND", "HORIZONTAL");
    IupSetAttribute(filterSelectList3, "VISIBLECOLUMNS", "15");
    IupSetAttribute(filterSelectList3, "DROPDOWN", "YES");
    IupSetCallback(filterSelectList3, "ACTION", (Icallback)uiList3SelectCb);

    IupSetAttribute(filterSelectList4, "VISIBLECOLUMNS", "4");
    IupSetAttribute(filterSelectList4, "DROPDOWN", "YES");
    IupSetCallback(filterSelectList4, "ACTION", (Icallback)uiList4SelectCb);

    IupSetAttribute(filterSelectList5, "VISIBLECOLUMNS", "2");
    IupSetAttribute(filterSelectList5, "DROPDOWN", "YES");
    IupSetCallback(filterSelectList5, "ACTION", (Icallback)uiList5SelectCb);

    // setup module uis
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        uiSetupModule(*(modules+ix), bottomVbox);
    }

    // dialog
    dialog = IupDialog(
        dialogVBox = IupVbox(
            topFrame,
            middleFrame,
            lowerMiddleFrame,
            bottomFrame,
            statusLabel,
            NULL
        )
    );

    IupSetAttribute(dialog, "TITLE", "clumsy Keybind Edition by mmaxx45 " CLUMSY_VERSION);
    IupSetAttribute(dialog, "SIZE", "480x"); // add padding manually to width
    IupSetAttribute(dialog, "RESIZE", "NO");
    IupSetCallback(dialog, "SHOW_CB", (Icallback)uiOnDialogShow);

    // global layout settings to affect childrens
    IupSetAttribute(dialogVBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(dialogVBox, "NCMARGIN", "4x4");
    IupSetAttribute(dialogVBox, "NCGAP", "4x2");

    // setup timer
    timer = IupTimer();
    IupSetAttribute(timer, "TIME", STR(ICON_UPDATE_MS));
    IupSetCallback(timer, "ACTION_CB", uiTimerCb);

    // setup timeout of program
    arg_value = IupGetGlobal("timeout");
    if(arg_value != NULL)
    {
        char valueBuf[16];
        sprintf(valueBuf, "%s000", arg_value);  // convert from seconds to milliseconds

        timeout = IupTimer();
        IupStoreAttribute(timeout, "TIME", valueBuf);
        IupSetCallback(timeout, "ACTION_CB", uiTimeoutCb);
        IupSetAttribute(timeout, "RUN", "YES");
    }
}

void startup() {
    // initialize seed
    srand((unsigned int)time(NULL));

    // kickoff event loops
    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
    IupMainLoop();
    // ! main loop won't return until program exit
}

void cleanup() {

    IupDestroy(timer);
    if (timeout) {
        IupDestroy(timeout);
    }

    IupClose();
    endTimePeriod(); // try close if not closing
}

// ui logics
void showStatus(const char *line) {
    IupStoreAttribute(statusLabel, "TITLE", line);
}

// in fact only 32bit binary would run on 64 bit os
// if this happens pop out message box and exit
static BOOL check32RunningOn64(HWND hWnd) {
    BOOL is64ret;
    // consider IsWow64Process return value
    if (IsWow64Process(GetCurrentProcess(), &is64ret) && is64ret) {
        MessageBox(hWnd, (LPCSTR)"You're running 32bit clumsy on 64bit Windows, which wouldn't work. Please use the 64bit clumsy version.",
            (LPCSTR)"Aborting", MB_OK);
        return TRUE;
    }
    return FALSE;
}

static BOOL checkIsRunning() {
    //It will be closed and destroyed when programm terminates (according to MSDN).
    HANDLE hStartEvent = CreateEventW(NULL, FALSE, FALSE, L"Global\\CLUMSY_IS_RUNNING_EVENT_NAME");

    if (hStartEvent == NULL)
        return TRUE;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hStartEvent);
        hStartEvent = NULL;
        return TRUE;
    }

    return FALSE;
}

static int uiOnDialogShow(Ihandle *ih, int state) {
    // only need to process on show
    HWND hWnd;
    BOOL exit;
    HICON icon;
    HINSTANCE hInstance;
    if (state != IUP_SHOW) return IUP_DEFAULT;
    hWnd = (HWND)IupGetAttribute(ih, "HWND");
    hInstance = GetModuleHandle(NULL);

    // set application icon
    icon = LoadIcon(hInstance, "CLUMSY_ICON");
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);

    exit = checkIsRunning();
    if (exit) {
        MessageBox(hWnd, (LPCSTR)"Theres' already an instance of clumsy running.",
            (LPCSTR)"Aborting", MB_OK);
        return IUP_CLOSE;
    }

#ifdef _WIN32
    exit = check32RunningOn64(hWnd);
    if (exit) {
        return IUP_CLOSE;
    }
#endif

    // try elevate and decides whether to exit
    exit = tryElevate(hWnd, parameterized);

    if (!exit && parameterized) {
        setFromParameter(filterText, "VALUE", "filter");
        LOG("is parameterized, start filtering upon execution.");
        uiStartCb(filterButton);
    }

    return exit ? IUP_CLOSE : IUP_DEFAULT;
}
static int uiStartCb(Ihandle* ih) {
    running = 1;
    char buf[MSG_BUFSIZE];
    UNREFERENCED_PARAMETER(ih);

    if (divertStart(IupGetAttribute(filterText, "VALUE"), buf) == 0) {
        showStatus(buf);
        return IUP_DEFAULT;
    }

    // Successfully started
    showStatus("Started filtering. Enable functionalities to take effect.");
    IupSetAttribute(filterText, "ACTIVE", "NO");
    IupSetAttribute(filterButton, "TITLE", "Stop");
    IupSetCallback(filterButton, "ACTION", uiStopCb);

    // Mode-specific timer handling
    if (Mode == 0) {
        IupSetAttribute(timer, "RUN", "NO");
    } else {
        IupSetAttribute(timer, "RUN", "YES");

        // Create and configure the delay timer
        Ihandle* delayTimer = IupTimer();
        intToStr(DelayTimerValue, timerbuffer);
        IupSetAttribute(delayTimer, "TIME", timerbuffer);
        IupSetCallback(delayTimer, "ACTION_CB", timerDelayCb);
        IupSetAttribute(delayTimer, "RUN", "YES");
    }

    return IUP_DEFAULT;
}
static int uiStopCb(Ihandle *ih) {
    running = 0;
    int ix;
    UNREFERENCED_PARAMETER(ih);

    // try stopping
    IupSetAttribute(filterButton, "ACTIVE", "NO");
    IupFlush(); // flush to show disabled state
    divertStop();

    IupSetAttribute(filterText, "ACTIVE", "YES");
    IupSetAttribute(filterButton, "TITLE", "Start");
    IupSetAttribute(filterButton, "ACTIVE", "YES");
    IupSetCallback(filterButton, "ACTION", uiStartCb);

    // stop timer and clean up icons
    IupSetAttribute(timer, "RUN", "NO");
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->processTriggered = 0; // use = here since is threads already stopped
        IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "none_icon");
    }
    sendState = SEND_STATUS_NONE;
    IupSetAttribute(stateIcon, "IMAGE", "none_icon");

    showStatus("Stopped. To begin again, edit criteria and click Start.");
    return IUP_DEFAULT;
}

static int uiToggleControls(Ihandle *ih, int state) {
    Ihandle *controls = (Ihandle*)IupGetAttribute(ih, CONTROLS_HANDLE);
    short *target = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    int controlsActive = IupGetInt(controls, "ACTIVE");
    if (controlsActive && !state) {
        IupSetAttribute(controls, "ACTIVE", "NO");
        InterlockedExchange16(target, I2S(state));
    } else if (!controlsActive && state) {
        IupSetAttribute(controls, "ACTIVE", "YES");
        InterlockedExchange16(target, I2S(state));
    }

    return IUP_DEFAULT;
}

static int uiTimerCb(Ihandle* ih) {
    int ix;
    UNREFERENCED_PARAMETER(ih);

    for (ix = 0; ix < MODULE_CNT; ++ix) {
        // Use Interlocked functions for thread-safe access
        short processTriggered = InterlockedExchange16(&(modules[ix]->processTriggered), 0);

        if (processTriggered) {
            IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "doing_icon");
        }
        else {
            IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "none_icon");
        }
    }
    return IUP_DEFAULT;
}

static int uiTimeoutCb(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    return IUP_CLOSE;
 }

static int uiListSelectCb(Ihandle* ih, char* text, int item, int state) {
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(ih);
    if (state == 1) {
        IupSetAttribute(filterText, "VALUE", filters[item - 1].filterValue);
    }
    return IUP_DEFAULT;
}

static int uiList2SelectCb(Ihandle* ih, char* text, int item, int state) {
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(state);
    NetworkType = item;

    return IUP_DEFAULT;
}

static int uiList4SelectCb(Ihandle* ih, char* text, int item, int state) {
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(state);

    if (state == 1) {
        if (strcmp(text, "Toggle") == 0) {
            IupHide(filterSelectList5);
            IupHide(TimerLabel);
            Mode = 0;
        }
        else if (strcmp(text, "Timer") == 0) {
            IupShow(filterSelectList5);
            IupShow(TimerLabel);
            Mode = 1;
        }
    }
    return IUP_DEFAULT;
}

static int uiList5SelectCb(Ihandle* ih, char* text, int item, int state) {
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(state);

    if (state == 1) {
        int seconds = atoi(text);  // Convert the text to an integer

        if (seconds >= 1 && seconds <= 60) {
            DelayTimerValue = seconds*1000;
        }
    }

    return IUP_DEFAULT;
}

static int uiList3SelectCb(Ihandle* ih, char* text, int item, int state) {
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(ih);

    if (state == 1 && item >= 1 && item <= 5) {
        currentPresetIndex = item - 1; // Update current preset index
        presetConfigFuncs[currentPresetIndex]();
    }
    return IUP_DEFAULT;
}

// Generic preset configuration function to eliminate code duplication
static void applyPresetConfig(const PresetConfig* preset) {
    // Lag
    Set_Lag_inboundCheckbox(preset->Lag_Inbound ? "ON" : "OFF");
    Set_Lag_outboundCheckbox(preset->Lag_Outbound ? "ON" : "OFF");
    Set_Lag_timeInput(preset->Lag_Delay);

    // Drop
    Set_Drop_inboundCheckbox(preset->Drop_Inbound ? "ON" : "OFF");
    Set_Drop_outboundCheckbox(preset->Drop_Outbound ? "ON" : "OFF");
    Set_Drop_chanceInput(preset->Drop_Chance);

    // Disconnect
    Set_Disconnect_inboundCheckbox(preset->Disconnect_Inbound ? "ON" : "OFF");
    Set_Disconnect_outboundCheckbox(preset->Disconnect_Outbound ? "ON" : "OFF");

    // Bandwidth
    Set_Bandwidth_inboundCheckbox(preset->BandwidthLimiter_Inbound ? "ON" : "OFF");
    Set_Bandwidth_outboundCheckbox(preset->BandwidthLimiter_Outbound ? "ON" : "OFF");
    Set_Bandwidth_bandwidthInput(preset->BandwidthLimiter_Limit);
    Set_Bandwidth_queueSizeInput(preset->BandwidthLimiter_QueueSize);
    Set_Bandwidth_speed(preset->BandwidthLimiter_Size);

    // Throttle
    Set_Throttle_inboundCheckbox(preset->Throttle_Inbound ? "ON" : "OFF");
    Set_Throttle_outboundCheckbox(preset->Throttle_Outbound ? "ON" : "OFF");
    Set_Throttle_frameInput(preset->Throttle_Timeframe);
    Set_Throttle_frameInpchanceInputut(preset->Throttle_Chance);
    Set_Throttle_dropThrottledCheckbox(preset->Throttle_DropThrottled ? "ON" : "OFF");

    // Duplicate
    Set_Duplicate_inboundCheckbox(preset->Duplicate_Inbound ? "ON" : "OFF");
    Set_Duplicate_outboundCheckbox(preset->Duplicate_Outbound ? "ON" : "OFF");
    Set_Duplicate_chanceInput(preset->Duplicate_Chance);
    Set_Duplicate_countInput(preset->Duplicate_Count);

    // OutOfOrder
    Set_OutOfOrder_inboundCheckbox(preset->OutOfOrder_Inbound ? "ON" : "OFF");
    Set_OutOfOrder_outboundCheckbox(preset->OutOfOrder_Outbound ? "ON" : "OFF");
    Set_OutOfOrder_chanceInput(preset->OutOfOrder_Chance);

    // Tamper
    Set_Tamper_inboundCheckbox(preset->Tamper_Inbound ? "ON" : "OFF");
    Set_Tamper_outboundCheckbox(preset->Tamper_Outbound ? "ON" : "OFF");
    Set_Tamper_chanceInput(preset->Tamper_Chance);
    Set_Tamper_checksumCheckbox(preset->Tamper_RedoChecksum ? "ON" : "OFF");

    // Reset
    Set_Reset_inboundCheckbox(preset->SetTCPRST_Inbound ? "ON" : "OFF");
    Set_Reset_outboundCheckbox(preset->SetTCPRST_Outbound ? "ON" : "OFF");
    Set_Reset_chanceInput(preset->SetTCPRST_Chance);
}

void preset1_config(void) {
    applyPresetConfig(&preset1);
}

void preset2_config(void) {
    applyPresetConfig(&preset2);
}
void preset3_config(void) {
    applyPresetConfig(&preset3);
}
void preset4_config(void) {
    applyPresetConfig(&preset4);
}
void preset5_config(void) {
    applyPresetConfig(&preset5);
}

// Switch to a specific preset and update UI
void switchPreset(int direction) {
    // Update preset index (direction: 1 = next, -1 = previous)
    currentPresetIndex = (currentPresetIndex + direction + 5) % 5;

    // Apply the new preset configuration to UI
    // This updates the UI elements and triggers callbacks that atomically
    // update the module variables, so changes apply instantly even while filtering
    presetConfigFuncs[currentPresetIndex]();

    // Update the dropdown UI to reflect the change (1-based index)
    char indexStr[8];
    sprintf(indexStr, "%d", currentPresetIndex + 1);
    IupSetAttribute(filterSelectList3, "VALUE", indexStr);

    // Print notification with status
    if (running) {
        printf("Switched to preset: %s (changes applied instantly)\n",
               presetLookup[currentPresetIndex]->PresetName);
    } else {
        printf("Switched to preset: %s\n", presetLookup[currentPresetIndex]->PresetName);
    }
}

static int uiFilterTextCb(Ihandle *ih)  {
    UNREFERENCED_PARAMETER(ih);
    // unselect list
    IupSetAttribute(filterSelectList, "VALUE", "0");
    return IUP_DEFAULT;
}

static void uiSetupModule(Module *module, Ihandle *parent) {
    Ihandle *groupBox, *toggle, *controls, *icon;
    groupBox = IupHbox(
        icon = IupLabel(NULL),
        toggle = IupToggle(module->displayName, NULL),
        IupFill(),
        controls = module->setupUIFunc(),
        NULL
    );
    IupSetAttribute(groupBox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(groupBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(controls, "ALIGNMENT", "ACENTER");
    IupAppend(parent, groupBox);

    // set controls as attribute to toggle and enable toggle callback
    IupSetCallback(toggle, "ACTION", (Icallback)uiToggleControls);
    IupSetAttribute(toggle, CONTROLS_HANDLE, (char*)controls);
    IupSetAttribute(toggle, SYNCED_VALUE, (char*)module->enabledFlag);
    IupSetAttribute(controls, "ACTIVE", "NO"); // startup as inactive
    IupSetAttribute(controls, "NCGAP", "4"); // startup as inactive

    // set default icon
    IupSetAttribute(icon, "IMAGE", "none_icon");
    IupSetAttribute(icon, "PADDING", "4x");
    module->iconHandle = icon;

    // parameterize toggle
    if (parameterized) {
        setFromParameter(toggle, "VALUE", module->shortName);
    }
}

// Thread function
DWORD WINAPI threadFunction(LPVOID lpParam) {
    UNREFERENCED_PARAMETER(lpParam);
    printf("Hello from the thread!\n");

    // Convert keybind once outside the loop for better performance
    wchar_t wch;
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, &wch, 2, general.Keybind, 1);

    if (convertedChars == 0) {
        return 1;
    }

    SHORT vkScan = VkKeyScanW(wch);
    int vkCode = LOBYTE(vkScan);

    if (vkScan == -1) {
        return 1;
    }

    BOOL keyToggled = FALSE; // Track the toggle state
    BOOL keyCurrentlyDown = FALSE; // Track the current key state

    while (1) {
        BOOL isKeyDown = (GetAsyncKeyState(vkCode) & 0x8000) != 0;
        BOOL isCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        BOOL isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

        if (isKeyDown && !keyCurrentlyDown) {
            // Key was just pressed
            keyCurrentlyDown = TRUE; // Set the key state to down

            // Check for Ctrl+Key (next preset) or Shift+Key (previous preset)
            if (isCtrlPressed) {
                printf("Ctrl + %s pressed: Switching to next preset\n", general.Keybind);
                switchPreset(1); // Next preset
            }
            else if (isShiftPressed) {
                printf("Shift + %s pressed: Switching to previous preset\n", general.Keybind);
                switchPreset(-1); // Previous preset
            }
            else {
                // Regular key press - toggle filtering
                keyToggled = !keyToggled; // Toggle the state
                printf("Key %s toggled to %s!\n", general.Keybind, keyToggled ? "ON" : "OFF");
                if (running) {
                    uiStopCb(filterButton);
                }
                else {
                    if (Mode == 0) {
                        uiStartCb(filterButton);
                    }
                    else {
                        IupSetAttribute(filterButton, "ACTIVE", "NO");
                        uiStartCb(filterButton);
                        Sleep(DelayTimerValue);
                        uiStopCb(filterButton);
                        IupSetAttribute(filterButton, "ACTIVE", "YES");
                    }
                }
            }
        }
        else if (!isKeyDown && keyCurrentlyDown) {
            // Key was just released
            keyCurrentlyDown = FALSE; // Set the key state to up
        }

        Sleep(10); // Sleep to reduce CPU usage
    }
    return 0;
}
// Callback function for the timer
static int timerDelayCb(Ihandle* ih) {
    // Call uiStopCb to stop filtering
    uiStopCb(ih);
    // Destroy the timer after use to clean up
    IupDestroy(ih);
    return IUP_DEFAULT;
}

void intToStr(int num, char* str) {
    sprintf(str, "%d", num);
}
int main(int argc, char* argv[]) {

    if (ini_parse("presets.ini", handler1, &preset1) < 0) {
        return 1;
    }
    if (ini_parse("presets.ini", handler2, &preset2) < 0) {
        return 1;
    }
    if (ini_parse("presets.ini", handler3, &general) < 0) {
        return 1;
    }
    if (ini_parse("presets.ini", handler4, &preset3) < 0) {
        return 1;
    }
    if (ini_parse("presets.ini", handler5, &preset4) < 0) {
        return 1;
    }
    if (ini_parse("presets.ini", handler6, &preset5) < 0) {
        return 1;
    }

    LOG("Config loaded from 'presets.ini': keybind=%c\n", general.Keybind);
    LOG("Is Run As Admin: %d", IsRunAsAdmin());
    LOG("Is Elevated: %d", IsElevated());
    init(argc, argv);

    HANDLE thread;
    DWORD threadId;

    // Create a new thread
    thread = CreateThread(NULL, 0, threadFunction, NULL, 0, &threadId);
    if (thread == NULL) {
        printf("Error creating thread\n");
        return 1;
    }

    // Calculate the length needed for the attributes string
    int len = snprintf(NULL, 0, "1=%s, 2=%s, 3=%s, 4=%s, 5=%s", preset1.PresetName, preset2.PresetName, preset3.PresetName, preset4.PresetName, preset5.PresetName) + 1;
    char* attributes = (char*)malloc(len);

    // Construct the attribute string
    snprintf(attributes, len, "1=%s, 2=%s, 3=%s, 4=%s, 5=%s", preset1.PresetName, preset2.PresetName, preset3.PresetName, preset4.PresetName, preset5.PresetName);

    // Set the attributes
    IupSetAttributes(filterSelectList3, attributes);
    IupSetAttributes(filterSelectList4, "1=Toggle, 2=Timer");
    IupSetAttributes(filterSelectList5, "1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, 8=8, 9=9, 10=10, 11=11, 12=12, 13=13, 14=14, 15=15, 16=16, 17=17, 18=18, 19=19, 20=20, 21=21, 22=22, 23=23, 24=24, 25=25, 26=26, 27=27, 28=28, 29=29, 30=30, 31=31, 32=32, 33=33, 34=34, 35=35, 36=36, 37=37, 38=38, 39=39, 40=40, 41=41, 42=42, 43=43, 44=44, 45=45, 46=46, 47=47, 48=48, 49=49, 50=50, 51=51, 52=52, 53=53, 54=54, 55=55, 56=56, 57=57, 58=58, 59=59, 60=60");

    IupSetAttribute(filterSelectList5, "VALUE", "1");

    preset1_config();

   // IupSetAttribute(filterSelectList3, "VALUE", "1");

    // Free the allocated memory
    free(attributes);
    startup();
    // Wait for the thread to finish
    //WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    cleanup();

    return 0;
}
