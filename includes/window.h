#ifndef _WINDOW_H
#define _WINDOW_H

#ifdef WAYLAND_SURFACE
#include <wayland-client.h>
#include <linux/input-event-codes.h>
#include "xdg-shell-client-protocol.h"
#include "utils.h"
#endif

enum mouse_buttons {
    MOUSE_BTN_LEFT = 1 << 0,
    MOUSE_BTN_RIGHT = 1 << 1,
    MOUSE_BTN_MIDDLE = 1 << 2
};

enum mouse_state_bitmask {
    MOUSE_STATE_HOVERING = 1 << 0,
    MOUSE_STATE_MOVING = 1 << 1, // not used for now because we need to track when it's not moving
    MOUSE_STATE_CLICK = 1 << 2
};

#define WINDOW_EDGE_MARGIN 5
enum window_edge_bitmask {
    WINDOW_EDGE_RIGHT = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT,
    WINDOW_EDGE_LEFT = XDG_TOPLEVEL_RESIZE_EDGE_LEFT,
    WINDOW_EDGE_TOP = XDG_TOPLEVEL_RESIZE_EDGE_TOP,
    WINDOW_EDGE_BOTTOM = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM
};

typedef struct mouse {
    int pos_x;
    int pos_y;
    int btn_clicked_bitmask;
    enum window_edge_bitmask edge_bitmask;

} * mouse_t;

typedef struct window
{
    bool should_close;
    int width;
    int height;
    int state_bitmask;
    struct mouse mouse;
    bool framebuffer_resized;

    #ifdef WAYLAND_SURFACE
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;

    struct wl_surface *surface;
    uint8_t *pool_data;

    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    struct wl_seat *seat;
    struct wl_pointer *pointer;
    // todo do touchpad
    #endif
} * window_t;

#endif
