#pragma once

#include <windows.h>
#include <tchar.h>

// Exit value.
const int ExitOK = 0;
const int ExitNG = 1;

// EventId
const int EventIdTimer = 1;
const int EventIdNextButton = 1000;

// Inteval
const int TimerInterval = 15;

// Window class name.
const TCHAR WindowClass[] = _T("RssHeadline");

// String that appears in the application's title bar.
static const TCHAR WindowTitle[] = _T("RSS Headline");

// Window size
const int WindowWidth = 1200;
const int WindowHeight = 70;

// Window color
const int WindowForeColorR = 255;
const int WindowForeColorG = 241;
const int WindowForeColorB = 0;
const int WindowBackColorR = 0;
const int WindowBackColorG = 0;
const int WindowBackColorB = 0;

// RSS position
const int ItemTop = 5;
const int ItemMin = -1500;
const int ItemLengthPerCharactor = 15;
const int ItemMargin = 100;

// Config filename
const TCHAR ConfigFilePath[] = _T("sites.xml");
