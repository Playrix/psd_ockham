/**
 * psd_ockham - Photoshop file size reducing utility
 * Copyright (C) 2018 Playrix.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "resources.h"

#include "libpsd_ockham.h"
#include "version.h"

#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <Richedit.h>

#define MAX_LOADSTRING 255
#define MAX_FILESTRING 2048

// Global Variables
HINSTANCE hInst;
CHAR szTitle[MAX_LOADSTRING];
CHAR szWindowClass[MAX_LOADSTRING];
HWND hWnd;
HWND hLogText;
HWND hCancelButton;

std::thread processingThread;
bool requestProcessingCancel = false;

static const std::vector<std::string> PhotoshopExtensions = {"psd", "psb"};

// Layout
static const int Border = 7;
static const int LabelHeight = 17;
static const int ButtonWidth = 138;
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
static void         AddToLogN(LogStatus status, const CHAR* text);
static void         AddToLogFromId(LogStatus status, int strId);
static void         AddToLogFromIdN(LogStatus status, int strId);
static void         SetLog(const CHAR* text);
static void         SetLogDefault();

static void         ProcessFiles(const std::vector<std::string>& files);
static void         StopProcessing();

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
	wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	DragAcceptFiles(hWnd, TRUE);

	CHAR buffer[MAX_LOADSTRING];

	HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	LoadLibrary("riched20.dll");
	hLogText = CreateWindowEx(
		WS_EX_STATICEDGE, RICHEDIT_CLASS,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
		0, 0, 0, 0,
		hWnd, (HMENU)ID_LOG_TEXT, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	SendMessage(hLogText, WM_SETFONT, (WPARAM)defaultFont, NULL);
	SetLogDefault();
	AddToLogFromId(LogStatus::None, IDS_LOG_DEFAULT);

	hCancelButton = CreateWindowEx(
		0, "BUTTON",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		0, 0, 0, 0,
		hWnd, (HMENU)ID_CANCEL_BUTTON, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	SendMessage(hCancelButton, WM_SETFONT, (WPARAM)defaultFont, NULL);
	EnableWindow(hCancelButton, FALSE);
	LoadString(hInstance, IDS_CANCEL, buffer, MAX_LOADSTRING);
	SendMessage(hCancelButton, WM_SETTEXT, FALSE, (LPARAM)buffer);

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
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case ID_CANCEL_BUTTON:
				if (!requestProcessingCancel)
				{
					requestProcessingCancel = true;
					AddToLogFromIdN(LogStatus::None, IDS_LOG_WAIT_TO_CANCEL);
				}
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

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
			lpMMI->ptMinTrackSize.x = 160;
			lpMMI->ptMinTrackSize.y = 160;
		}
		break;

	
	case WM_DESTROY:
		StopProcessing();
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

			if (processingThread.joinable())
				processingThread.join();
			processingThread = std::thread(ProcessFiles, files);
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

	const int logHeight = windowHeight - y - Border - Gap - ButtonHeight;

	MoveWindow(hLogText,
		Border, y,
		windowWidth - Border*2,
		logHeight,
		TRUE
	);

	RECT rc;
	SendMessage(hLogText, EM_GETRECT, 0, (LPARAM)&rc);
	rc.left = TextMarginH;
	rc.top = TextMarginV;
	SendMessage(hLogText, EM_SETRECT, 0, (LPARAM)&rc);

	y += logHeight + Gap;

	MoveWindow(hCancelButton,
		windowWidth - Border - ButtonWidth, y,
		ButtonWidth,
		ButtonHeight,
		TRUE
	);
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

static void AddToLogFromIdN(LogStatus status, int strId)
{
	AddToLogFromId(status, strId);
	AddToLog(status, "\r\n");
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

static void SetLogDefault()
{
	CHAR buffer[MAX_LOADSTRING];
	LoadString(hInst, IDS_APP_VERSION, buffer, MAX_LOADSTRING);
	std::string version = std::string("v") + PSD_OCKHAM_VERSION + "+" + buffer;
	SetLog("");
	AddToLog(LogStatus::None, version.c_str());
	AddToLog(LogStatus::None, "\r\n\r\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Processing psd files logic

static void StopProcessing()
{
	if (processingThread.joinable())
	{
		::TerminateThread(processingThread.native_handle(), 1);
		processingThread.join();
	}
}

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
	requestProcessingCancel = false;

	std::vector<std::string> psdFiles;

	FilterFiles(files, psdFiles);

	SetLogDefault();

	if (psdFiles.empty())
	{
		AddToLogFromId(LogStatus::Warning, IDS_LOG_NO_FILES);
		return;
	}

	DragAcceptFiles(hWnd, FALSE);
	EnableWindow(hCancelButton, TRUE);

	if (psdFiles.size() > 1)
	{
		AddToLogFromIdFormattedN(LogStatus::None, IDS_LOG_NUM_FILES, psdFiles.size());
	}

	bool hasErrors = false;
	char * error_message = new char[2048];
	size_t i = 0;
	for (i = 0; i < psdFiles.size(); ++i)
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

		if (requestProcessingCancel)
			break;
	}
	delete [] error_message;

	if (!requestProcessingCancel || i+1 >= psdFiles.size())
	{
		if (hasErrors)
			AddToLogFromId(LogStatus::Warning, IDS_LOG_DONE_ERRORS);
		else
			AddToLogFromId(LogStatus::Success, IDS_LOG_DONE);
	}
	else
	{
		AddToLogFromId(LogStatus::Warning, IDS_LOG_CANCEL);
	}

	DragAcceptFiles(hWnd, TRUE);
	EnableWindow(hCancelButton, FALSE);
}


