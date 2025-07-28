#ifndef _WINDOW_H
    #define _WINDOW_H

    #include <stdbool.h>

    #include "utils.h"

    #ifdef WAYLAND_SURFACE
        #include <wayland-client.h>
        #include <linux/input-event-codes.h>

        #include <xdg/xdg-shell-client-protocol.h>
        #include <xdg/xdg-decoration-unstable-v1-client-protocol.h>
    #endif

enum mouse_buttons {
    MOUSE_BTN_LEFT = 1 << 0,
    MOUSE_BTN_RIGHT = 1 << 1,
    MOUSE_BTN_MIDDLE = 1 << 2
};

enum mouse_state_bitmask {
    MOUSE_STATE_HOVERING = 1 << 0,
    MOUSE_STATE_MOVING_WINDOW = 1 << 1, // not used for now because we need to track when it's not moving
    MOUSE_STATE_CLICK = 1 << 2
};

/**
 * @def WINDOW_EDGE_MARGIN
 * @brief Define the margin of error for the detection of the window edge 
 */
#define WINDOW_EDGE_MARGIN 10.f

enum window_edge_bitmask {
    #ifdef WAYLAND_SURFACE
    WINDOW_EDGE_RIGHT = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT,
    WINDOW_EDGE_LEFT = XDG_TOPLEVEL_RESIZE_EDGE_LEFT,
    WINDOW_EDGE_TOP = XDG_TOPLEVEL_RESIZE_EDGE_TOP,
    WINDOW_EDGE_BOTTOM = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM
    #else
    WINDOW_EDGE_RIGHT = 1 << 0,
    WINDOW_EDGE_LEFT = 1 << 1,
    WINDOW_EDGE_TOP = 1 << 2,
    WINDOW_EDGE_BOTTOM = 1 << 3
    #endif
};

/**
 * @struct mouse
 * @brief structure representing the mouse device
 * @var mouse::pos_x
 * Position x of the mouse
 * @var mouse::pos_y
 * Position y of the mouse
 * @var mouse::btn_clicked_bitmask
 * Bitmask representing the state of mouse buttons, using the `enum mouse_buttons`
 * @var mouse::edge_bitmask
 * Bitmask representing on wich edge the mouse is, using the `enum window_edge_bitmask`
 * @var mouse::mouse_state_bitmask
 * Bitmask representing the state of the mouse, using the `enum mouse_state_bitmask`
 */
typedef struct mouse {
    double pos_x;
    double pos_y;
    enum mouse_buttons btn_clicked_bitmask;
    enum window_edge_bitmask edge_bitmask;
    enum mouse_state_bitmask mouse_state_bitmask;

} * mouse_t;

/**
 * @struct window
 * @brief structure representing the window and its properties
 * @var window::should_close
 * Boolean specifying if the window should close or not due to internal events triggered
 * @var window::width
 * Width of the window
 * @var window::height
 * Height of the window
 * @var window::mouse
 * Mouse device
 * @var window::framebuffer_resize
 * Boolean specifying if the framebuffer should be resized
 * @var window::title
 * Title of the window
 */
typedef struct window
{
    bool should_close;
    int width;
    int height;
    struct mouse mouse;
    bool framebuffer_resized;
    const char *title;

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

    //struct zxdg_decoration_manager_v1 *zxdg_decoration_manager;
    //struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration;
    #endif
} * window_t;

#endif
