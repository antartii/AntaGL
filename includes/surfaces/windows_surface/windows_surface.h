#ifndef _WINDOWS_SURFACE_H
#define _WINDOWS_SURFACE_H

#ifndef UNICODE
#define UNICODE
#endif 

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

#define CLASS_NAME L"AntaGLWindow"

typedef struct windows_surface_context {
    HWND hwnd;
    HINSTANCE handle;
} * windows_surface_context_t;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool test_windows(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);

bool end_windows_surface(windows_surface_context_t surface);
bool init_windows_surface(windows_surface_context_t surface, window_t window);
bool poll_events_windows_surface(windows_surface_context_t surface);

extern const char *windows_surface_instance_extensions[];

#endif