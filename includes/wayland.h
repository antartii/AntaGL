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

#include "xdg-shell-client-protocol.h"

/*

Every message between client and server is in 32-bit words
Header has 2 words:
    - Sender's object id (32-bit)
    - Message size (upper 16-bit) + Request/Event opcode (lower 16-bit)

*/

struct window
{
    // wayland
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;

    struct wl_surface *surface;
    struct wl_shm *shm;
    uint8_t *pool_data;

    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
};

bool init_wayland(struct window *window);
bool end_wayland(struct window *window);

#endif
