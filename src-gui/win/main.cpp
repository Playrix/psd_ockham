// psd_ockham_gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resources.h"

#include <vector>
#include <string>

#define MAX_LOADSTRING 255
#define MAX_FILESTRING 2048

// Global Variables
HINSTANCE hInst;
CHAR szTitle[MAX_LOADSTRING];
CHAR szWindowClass[MAX_LOADSTRING];
HWND hVersionLabel;
HWND hLogText;

// Layout
const int border = 7;
const int labelHeight = 17;
const int buttonHeight = 30;
const int gap = 3;
const int textMarginV = 3;
const int textMarginH = 5;

// Forward declarations
ATOM                OckhamRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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

    hLogText = CreateWindowEx(
        WS_EX_STATICEDGE, "EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY,
        0, 0, 0, 0,
        hWnd, (HMENU)ID_LOG_TEXT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
    SendMessage(hLogText, WM_SETFONT, (WPARAM)defaultFont, NULL);
    CHAR szDefaultText[MAX_LOADSTRING];
    LoadString(hInstance, IDS_LOG_DEFAULT_TEXT, szDefaultText, MAX_LOADSTRING);
    SendMessage(hLogText, WM_SETTEXT, FALSE, (LPARAM)szDefaultText);


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

static void AddToLogText(const CHAR* text)
{
    int outLength = GetWindowTextLength(hLogText);
    SendMessage(hLogText, EM_SETSEL, outLength, outLength);
    SendMessage(hLogText, EM_REPLACESEL, FALSE, (LPARAM)text);
}

static void SetLogText(const CHAR* text)
{
    SendMessage(hLogText, WM_SETTEXT, FALSE, (LPARAM)text);
}

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
            std::vector<std::basic_string<CHAR>> files;
            for (UINT i = 0; i < count; ++i)
            {
                if (DragQueryFile(hDrop, i, (LPSTR)&buf, MAX_FILESTRING) != 0)
                {
                    files.push_back(std::basic_string<CHAR>(buf));
                }
            }

            DragFinish(hDrop);

            SetLogText("");

            for (auto it: files)
            {
                AddToLogText(it.c_str());
                AddToLogText("\r\n");
            }
        }
        break;

    case WM_SIZE:
        {
            int y = border;

            SIZE sizeText;
            CHAR buf[MAX_LOADSTRING];
            GetWindowText(hVersionLabel, buf, MAX_LOADSTRING);
            HDC hDC = GetDC(NULL);
            HFONT childFont = (HFONT)SendMessage(hVersionLabel, WM_GETFONT, 0,0);
            HGDIOBJ oldFont = SelectObject(hDC, childFont);
            GetTextExtentPoint32(hDC, buf, lstrlen(buf), &sizeText);
            SelectObject(hDC, oldFont);

            MoveWindow(hVersionLabel,
                        LOWORD(lParam) - border - sizeText.cx, y,
                        sizeText.cx,
                        labelHeight,
                        TRUE);

            y += labelHeight;
            y += gap;

            MoveWindow(hLogText,
                        border, y,
                        LOWORD(lParam) - border*2,
                        HIWORD(lParam) - y - border,
                        TRUE);

            RECT rc;
            SendMessage(hLogText, EM_GETRECT, 0, (LPARAM)&rc);
            rc.left += textMarginH;
            rc.top += textMarginV;
            SendMessage(hLogText, EM_SETRECT, 0, (LPARAM)&rc);
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
