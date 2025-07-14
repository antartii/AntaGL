#ifndef _WINDOW_H
#define _WINDOW_H

#ifdef WAYLAND_SURFACE
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#endif

enum window_states {
    MAXIMIZED,
    FULLSCREEN,
    RESIZING,
    ACTIVATED,

    TILED_LEFT,
    TILED_RIGHT,
    TILED_TOP,
    TILED_BOTTOM
};

struct window
{
    bool should_close;
    int width;
    int height;
    enum window_states state;

    #ifdef WAYLAND_SURFACE
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;

    struct wl_surface *surface;
    uint8_t *pool_data;

    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    #endif
};

typedef struct window * window_t;

#endif
