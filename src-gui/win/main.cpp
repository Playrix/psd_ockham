// psd_ockham_gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resources.h"

#include "libpsd_ockham.h"

#include <vector>
#include <string>
#include <algorithm>

#include "Richedit.h"

#define MAX_LOADSTRING 255
#define MAX_FILESTRING 2048

// Global Variables
static HINSTANCE hInst;
static CHAR szTitle[MAX_LOADSTRING];
static CHAR szWindowClass[MAX_LOADSTRING];
static HWND hVersionLabel;
static HWND hLogText;

static const std::vector<std::string> PhotoshopExtensions = {"psd", "psb"};

// Layout
static const int Border = 7;
static const int LabelHeight = 17;
static const int ButtonHeight = 30;
static const int Gap = 3;
static const int TextMarginV = 3;
static const int TextMarginH = 5;

enum class LogStatus
{
	None,
	Success,
	Warning,
	Error,
};

// Forward declarations
ATOM                OckhamRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

static void         LayoutElements(int windowWidth, int windowHeight);
static void         AddToLog(LogStatus status, const CHAR* text);
static void         AddToLogFromId(LogStatus status, int strId);
static void         SetLog(const CHAR* text);

static void         ProcessFiles(const std::vector<std::string>& files);

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Initialization

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINDOW_CLASS, szWindowClass, MAX_LOADSTRING);
    OckhamRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM OckhamRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = wcex.hIcon;

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindow(szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
        CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    DragAcceptFiles(hWnd, TRUE);

    HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    hVersionLabel = CreateWindowEx(
        0, "EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | ES_READONLY,
        0, 0, 0, 0,
        hWnd, (HMENU)ID_VERSION_LABEL, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
    SendMessage(hVersionLabel, WM_SETFONT, (WPARAM)defaultFont, NULL);
    CHAR szVersion[MAX_LOADSTRING];
    LoadString(hInstance, IDS_APP_VERSION, szVersion, MAX_LOADSTRING);
    SendMessage(hVersionLabel, WM_SETTEXT, FALSE, (LPARAM)szVersion);

    LoadLibrary("riched20.dll");
    hLogText = CreateWindowEx(
        WS_EX_STATICEDGE, RICHEDIT_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
        0, 0, 0, 0,
        hWnd, (HMENU)ID_LOG_TEXT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
    SendMessage(hLogText, WM_SETFONT, (WPARAM)defaultFont, NULL);
    SetLog("");
    AddToLogFromId(LogStatus::None, IDS_LOG_DEFAULT);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main messages routine

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    // case WM_COMMAND:
    //     {
    //         int wmId = LOWORD(wParam);
    //         // Parse the menu selections:
    //         switch (wmId)
    //         {
    //         case IDM_ABOUT:
    //             DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
    //             break;
    //         default:
    //             return DefWindowProc(hWnd, message, wParam, lParam);
    //         }
    //     }
    //     break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: Add any drawing code that uses hdc here...

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 150;
            lpMMI->ptMinTrackSize.y = 100;
        }
        break;

    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_DROPFILES:
        {
            HDROP hDrop = (HDROP)wParam;

            UINT count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            CHAR buf[MAX_FILESTRING];
            std::vector<std::string> files;
            for (UINT i = 0; i < count; ++i)
            {
                if (DragQueryFile(hDrop, i, (LPSTR)&buf, MAX_FILESTRING) != 0)
                {
                    files.push_back(std::string(buf));
                }
            }

            DragFinish(hDrop);

            ProcessFiles(files);
        }
        break;

    case WM_SIZE:
        {
            LayoutElements(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            if ((HWND)lParam == hLogText)
            {
                return (LONG)GetStockObject(WHITE_BRUSH);
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Misc layout functions

static void LayoutElements(int windowWidth, int windowHeight)
{
    int y = Border;

    SIZE sizeText;
    CHAR buf[MAX_LOADSTRING];
    GetWindowText(hVersionLabel, buf, MAX_LOADSTRING);
    HDC hDC = GetDC(NULL);
    HFONT versionLabelFont = (HFONT)SendMessage(hVersionLabel, WM_GETFONT, 0,0);
    HGDIOBJ oldFont = SelectObject(hDC, versionLabelFont);
    GetTextExtentPoint32(hDC, buf, lstrlen(buf), &sizeText);
    SelectObject(hDC, oldFont);

    MoveWindow(hVersionLabel,
        windowWidth - Border - sizeText.cx, y,
        sizeText.cx,
        LabelHeight,
        TRUE
    );

    y += LabelHeight;
    y += Gap;

    MoveWindow(hLogText,
        Border, y,
        windowWidth - Border*2,
        windowHeight - y - Border,
        TRUE
    );

    RECT rc;
    SendMessage(hLogText, EM_GETRECT, 0, (LPARAM)&rc);
    rc.left = TextMarginH;
    rc.top = TextMarginV;
    SendMessage(hLogText, EM_SETRECT, 0, (LPARAM)&rc);
}

static COLORREF GetTextColor(LogStatus status)
{
    switch (status)
    {
        case LogStatus::Success:
            return RGB(0, 128, 0);
            
        case LogStatus::Warning:
            return RGB(255, 128, 0);
            
        case LogStatus::Error:
            return RGB(255, 0, 0);
            
        default:
            return RGB(0, 0, 0);
    }
}

static void AddToLog(LogStatus status, const CHAR* text)
{
    int outLength = GetWindowTextLength(hLogText);
    SendMessage(hLogText, EM_SETSEL, outLength, outLength);

    CHARFORMAT cf = {0};
    SendMessage(hLogText, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = GetTextColor(status);

    SendMessage(hLogText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hLogText, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(hLogText, WM_VSCROLL, SB_BOTTOM, 0);
}

static void AddToLogN(LogStatus status, const CHAR* text)
{
    AddToLog(status, text);
    AddToLog(status, "\r\n");
}

static void AddToLogFromId(LogStatus status, int strId)
{
    CHAR text[MAX_LOADSTRING];
    LoadString(hInst, strId, text, MAX_LOADSTRING);
    AddToLog(status, text);
}

template<typename... Types>
static void AddToLogFromIdFormatted(LogStatus status, int strId, Types ... args)
{
    CHAR pattern[MAX_LOADSTRING];
    LoadString(hInst, strId, pattern, MAX_LOADSTRING);
    CHAR msg[MAX_FILESTRING];
    sprintf_s(msg, MAX_FILESTRING, pattern, args...);
    AddToLog(status, msg);
}

template<typename... Types>
static void AddToLogFromIdFormattedN(LogStatus status, int strId, Types ... args)
{
    AddToLogFromIdFormatted(status, strId, args...);
    AddToLog(status, "\r\n");
}

static void SetLog(const CHAR* text)
{
    SendMessage(hLogText, WM_SETTEXT, FALSE, (LPARAM)text);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Processing psd files logic

static std::vector<std::string> ListDirectory(const std::string& dir)
{
    std::vector<std::string> result;
    std::string search_path = dir + "\\*.*";
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        std::string fileName;
        do
        {
            fileName = fd.cFileName;
            if (fileName != "." && fileName != "..")
            {
                result.push_back(dir + "\\" + fileName);
            }
        } while(::FindNextFile(hFind, &fd));

        ::FindClose(hFind);
    }

    return result;
}

static char ToLower(char in)
{
    if (in <='Z' && in >= 'A')
        return in-('Z'-'z');
    return in;
}

static void FilterFiles(const std::vector<std::string>& files, std::vector<std::string>& result)
{
    for (auto it: files)
    {
        DWORD attr = GetFileAttributes(it.c_str());
        if (attr & FILE_ATTRIBUTE_DIRECTORY)
        {
            FilterFiles(ListDirectory(it), result);
        }
        else
        {
            size_t ext_pos = it.find_last_of(".");
            if (ext_pos != std::string::npos)
            {
                std::string ext = it.substr(ext_pos + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ToLower);
                if (std::find(PhotoshopExtensions.begin(), PhotoshopExtensions.end(), ext) != PhotoshopExtensions.end())
                {
                    result.push_back(it);
                }
            }
        }
    }
}

static void ProcessFiles(const std::vector<std::string>& files)
{
    std::vector<std::string> psdFiles;

    FilterFiles(files, psdFiles);

    SetLog("");

    if (psdFiles.empty())
    {
        AddToLogFromId(LogStatus::Warning, IDS_LOG_NO_FILES);
        return;
    }

    if (psdFiles.size() > 1)
    {
        AddToLogFromIdFormattedN(LogStatus::None, IDS_LOG_NUM_FILES, psdFiles.size());
    }

    bool hasErrors = false;
    char * error_message = new char[2048];
    for (size_t i = 0; i < psdFiles.size(); ++i)
    {
        const char* file = psdFiles[i].c_str();

        AddToLogFromIdFormattedN(LogStatus::None, IDS_LOG_FILE_PROCESSING, i+1, psdFiles.size(), file);

        psd_result result = psd_process_file(file, NULL);
        if (result.status != psd_status_done)
        {
            psd_get_error_message(error_message, result.status, result.out_file);
            AddToLogN(LogStatus::Error, error_message);
            hasErrors = true;
        }
        if (result.out_file != NULL)
            delete [] result.out_file;
    }
    delete [] error_message;

    if (hasErrors)
        AddToLogFromId(LogStatus::Warning, IDS_LOG_DONE_ERRORS);
    else
        AddToLogFromId(LogStatus::Success, IDS_LOG_DONE);
}


