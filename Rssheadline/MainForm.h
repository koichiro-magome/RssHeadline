#pragma once
#include <windows.h>

/// Processes messages for the main window
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/// Procedure on creare
void OnCreate(HWND, int);

/// Procedure on destroy
void OnDestroy(HWND);

/// Procedure on L button down
void OnLButtonDown(HWND);

/// Procedure on R button down
void OnRButtonDown(HWND);

/// Procedure on paint
void OnPaint(HWND, int, int, int);

/// Procedure on timer
void OnTimer();
