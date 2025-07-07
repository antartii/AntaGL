#ifndef _WAYLAND_H
#define _WAYLAND_H

#include <stdio.h>
#include <wayland-client.h>
#include <stdbool.h>
#include <unistd.h>

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
};

bool init_wayland(struct window *window);

bool end_wayland(struct window *window);

#endif
