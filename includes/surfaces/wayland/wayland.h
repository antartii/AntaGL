#ifndef _WAYLAND_H
    #define _WAYLAND_H

    #include <stdio.h>
    #include <wayland-client.h>
    #include <stdbool.h>
    #include <unistd.h>
    #include <stdint.h>
    #include <string.h>
    #include <time.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <errno.h>

    #include <xdg/xdg-shell-client-protocol.h>
    #include <linux/input-event-codes.h>
    #include <xdg/xdg-decoration-unstable-v1-client-protocol.h>

    #define VK_USE_PLATFORM_WAYLAND_KHR

    #include "window.h"

typedef struct wayland_context {
    window_t user_window;

    enum xdg_toplevel_resize_edge resize_edge_bitmask;

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
} * wayland_context_t;

/**
 * @brief Array of the names for the extensions needed by the instance to use wayland
 */
extern const char *wayland_instance_extensions[];

/**
 * @def SURFACE_EXTENSIONS_NAMES 
 * @brief names for the wayland extensions
 */
#define SURFACE_EXTENSIONS_NAMES wayland_instance_extensions
/**
 * @def SURFACE_EXTENSIONS_COUNT
 * @brief Total count of extensions stored in SURFACE_EXTENSIONS_NAMES
 */
#define SURFACE_EXTENSIONS_COUNT 2

/**
 * @brief Initialize wayland and a window using wayland
 * 
 * @param window Pointer to the window to update
 * @return true if the initialisation didn't encountered a problem
 * @return false if the initialisation encountered a problem
 */
bool init_wayland(wayland_context_t context, window_t window);
/**
 * @brief End and destroy variables initialised by wayland
 *
 * @param window Pointer to the window where waylands variables will be destroyed
 * @return true if wayland ended without error
 * @return false if wayland encountered and error
 */
bool end_wayland(wayland_context_t context);

bool poll_events_wayland(wayland_context_t context);

#endif
