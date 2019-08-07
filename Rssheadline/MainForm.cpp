#include <stdlib.h>
#include <locale.h>
#include <string>

#include "FormConstant.h"
#include "ConfigParser.h"
#include "HttpClient.h"
#include "RssParser.h"
#include "MainForm.h"

// Instance
auto hInst = (HINSTANCE)nullptr;
auto hDCMemory = (HDC)nullptr;
std::vector<Common::ConfigItem> ConfigItems;
auto CurrentConfigIndex = 0U;
std::vector<Common::RssItem> RssItems;
auto IsInializing = false;

/// Entry point
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	// UTF-8
	SetConsoleOutputCP(CP_UTF8);

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) {
		MessageBox(nullptr, _T("RegisterClassEx failed"), WindowTitle, MB_OK | MB_ICONWARNING);
		return ExitNG;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
		WS_EX_COMPOSITED,
		WindowClass,											// the name of the application
		WindowTitle,											// the text that appears in the title bar
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_DLGFRAME,	// the type of window to create
		CW_USEDEFAULT,											// initial position (x)
		CW_USEDEFAULT,											// initial position (y)
		WindowWidth,											// initial size (width)
		WindowHeight,											// initial size (height)
		nullptr,												// the parent of this window
		nullptr,												// this application does not have a menu bar
		hInstance,												// the first parameter from WinMain
		nullptr													// not used in this application
	);

	if (!hWnd) {
		MessageBox(nullptr, _T("CreateWindowEx failed"), WindowTitle, MB_OK | MB_ICONWARNING);
		return ExitNG;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// Main message loop:
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE:

		OnCreate(hWnd, TimerInterval);

		break;

	case WM_DESTROY:

		OnDestroy(hWnd);

		PostQuitMessage(ExitOK);

		break;

	case WM_LBUTTONDOWN:

		OnLButtonDown(hWnd);

		break;

	case WM_RBUTTONDOWN:

		OnPaint(hWnd, WindowBackColorR, WindowBackColorG, WindowBackColorB);

		OnRButtonDown(hWnd);

		break;

	case WM_PAINT:

		OnPaint(hWnd, WindowForeColorR, WindowForeColorG, WindowForeColorB);

		break;

	case WM_QUERYNEWPALETTE:

		break;

	case WM_TIMER:

		OnTimer();
		
		// Refresh
		InvalidateRect(hWnd, nullptr, false);
		UpdateWindow(hWnd);

		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);

		break;
	}

	return 0;
}

void OnCreate(HWND hWnd, int interval) {

	if (ConfigItems.empty()) {

		// Parse Rss
		auto configParser = new Common::ConfigParser();
		ConfigItems = configParser->ParseConfig((TCHAR*)ConfigFilePath);
	}

	// Get desktop size
	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);

	// Get window DC
	auto hDCWindow = GetDC(hWnd);
	
	// Create bitmap
	auto hBitmap = CreateCompatibleBitmap(hDCWindow, rect.right, rect.bottom);
	
	// Create memory DC
	hDCMemory = CreateCompatibleDC(nullptr);

	// Copy bitmap to memory DC
	SelectObject(hDCMemory, hBitmap);

	// Initialize timer
	SetTimer(hWnd, EventIdTimer, interval, nullptr);
}

void OnDestroy(HWND hWnd) {

	// Destroy timer
	KillTimer(hWnd, EventIdTimer);
}

void OnTimer() {

	if (IsInializing) {
		return;
	}
	else
	{
		if (RssItems.empty()) {

			IsInializing = true;

			// Receive Http
			auto httpClient = new Common::HttpClient();

			size_t retUrlSize = 0U;
			char retUrlString[Common::UrlMaxSize];
			auto retVal = wcstombs_s(
				&retUrlSize,
				retUrlString,
				Common::UrlMaxSize,
				ConfigItems.at(CurrentConfigIndex).GetUrl(),
				Common::UrlMaxSize);

			std::string rssString = httpClient->Receive(retUrlString);

			// Parse Rss
			auto rssParser = new Common::RssParser();
			RssItems = rssParser->ParseRss(rssString);

			// Set first initial position
			auto initialPosition = WindowWidth;

			// About all xml items
			for (auto index = 0U; index < RssItems.size(); index++) {

				// Set position
				RssItems.at(index).SetPosition(initialPosition);

				// Set length
				auto length = (int)_tcslen(RssItems.at(index).GetTitle());
				RssItems.at(index).SetLength(length * ItemLengthPerCharactor + ItemMargin);

				// Set next initial position
				initialPosition += length * ItemLengthPerCharactor + ItemMargin;
			}

			delete rssParser;
			delete httpClient;

			IsInializing = false;
		}

		auto isDeleted = false;

		// About all xml items
		for (auto index = 0U; index < RssItems.size(); index++) {

			// Move text position
			auto position = RssItems.at(index).GetPosition() - 1;
			RssItems.at(index).SetPosition(position);

			// Delete head item
			if (position <= ItemMin) {

				// Delete title
				delete RssItems.at(0).GetTitle();

				// Delete link
				delete RssItems.at(0).GetLink();

				// Pop head item
				RssItems.erase(RssItems.begin());

				isDeleted = true;
			}
		}

		if (isDeleted && RssItems.empty()) {

			// Increment current config index
			CurrentConfigIndex++;
			if (CurrentConfigIndex >= ConfigItems.size()) {
				CurrentConfigIndex = 0;
			}
		}
	}
}

void OnLButtonDown(HWND hWnd) {

	// Get mouse position
	POINT pt;
	GetCursorPos(&pt);
	
	// Get window position
	RECT rect;
	GetWindowRect(hWnd, &rect);

	auto clickItemleft = 0;
	auto clickItemRight = 0;
	auto clickItemIndex = 0;
	auto* clickItemUrl = (TCHAR*)nullptr;
	auto flag = false;

	// About all xml items
	for (auto index = 0U; index < RssItems.size(); index++) {
		
		// Serach clicked link
		clickItemleft = RssItems.at(index).GetPosition();
		clickItemRight = RssItems.at(index).GetPosition() + RssItems.at(index).GetLength();

		if ((pt.x - rect.left) >= clickItemleft && (pt.x - rect.left) <= clickItemRight) {
			clickItemIndex = index;
			clickItemUrl = RssItems.at(index).GetLink();
			flag = true;
			break;
		}
	}

	// ----------------------------------------
	//TCHAR message[512];
	//wsprintf(message, L"x = %d, index = %d, left = %d, right = %d, link = %s",
	//	(pt.x - rect.left),
	//	clickItemIndex,
	//	clickItemleft,
	//	clickItemRight,
	//	clickItemUrl);
	//MessageBox(nullptr, message, WindowTitle, MB_OK);
	// ----------------------------------------

	// Open browser
	if (flag) {
		ShellExecute(NULL, _T("open"), clickItemUrl, NULL, _T(""), SW_SHOW);
	}
}

void OnRButtonDown(HWND hWnd) {

	if (IsInializing) {
		return;
	}
	else
	{
		if (!RssItems.empty()) {

			IsInializing = true;

			// About all xml items
			for (auto index = 0U; index < RssItems.size(); index++) {

				// Move text position
				RssItems.at(index).SetPosition(ItemMin);
			}

			IsInializing = false;
		}
	}
}

void OnPaint(HWND hWnd, int windowForeColorR, int windowForeColorG, int windowForeColorB) {

	// Fore color
	SetTextColor(hDCMemory, RGB(windowForeColorR, windowForeColorG, windowForeColorB));
	
	// BackColor
	SetBkColor(hDCMemory, RGB(WindowBackColorR, WindowBackColorG, WindowBackColorB));

	// About all xml items
	for (auto index = 0U; index < RssItems.size(); index++) {

		// Print text to DC memory
		TextOut(hDCMemory, RssItems.at(index).GetPosition(), ItemTop, RssItems.at(index).GetTitle(), (int)_tcslen(RssItems.at(index).GetTitle()));
	}

	// Get desktop size
	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);

	// Copy memory DC to window DC
	PAINTSTRUCT ps;
	auto hDCWindow = BeginPaint(hWnd, &ps);
	BitBlt(hDCWindow, 0, 0, rect.right, rect.bottom, hDCMemory, 0, 0, SRCCOPY);
	EndPaint(hWnd, &ps);
}
