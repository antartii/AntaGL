#include "windows_surface.h"

const char *windows_surface_instance_extensions[] = {
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

            EndPaint(hwnd, &ps);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

HMODULE GetCurrentModuleHandle() {
    HMODULE ImageBase;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&GetCurrentModuleHandle, &ImageBase)) {
        return ImageBase;
    }
    return 0;
}

bool end_windows_surface(windows_surface_context_t surface)
{
    DestroyWindow(surface->hwnd);
    UnregisterClass(CLASS_NAME, surface->handle);
    return true;
}

bool init_windows_surface(windows_surface_context_t surface, window_t window)
{
    const wchar_t class_name[] = CLASS_NAME;
    HINSTANCE hInstance = GetCurrentModuleHandle();
    size_t title_size = MultiByteToWideChar(CP_UTF8, 0, window->title, -1, NULL, 0);
    if (title_size == 0)
        return false;
    wchar_t *title = malloc(sizeof(wchar_t) * title_size);
    MultiByteToWideChar(CP_UTF8, 0, window->title, -1, title, title_size);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    RegisterClass(&wc);

    surface->handle = hInstance;
    surface->hwnd = CreateWindowEx(
        0,
        class_name,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);
    
    if (!surface->hwnd)
        return false;

    ShowWindow(surface->hwnd, SW_SHOWDEFAULT);

    return true;
}

bool poll_events_windows_surface(windows_surface_context_t surface)
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
