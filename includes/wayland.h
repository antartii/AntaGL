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

#include "window.h"
#include "xdg-shell-client-protocol.h"

extern const char *wayland_instance_extensions[];

#define WAYLAND_EXTENSIONS_NAMES wayland_instance_extensions
#define WAYLAND_EXTENSIONS_COUNT 2

/*

Every message between client and server is in 32-bit words
Header has 2 words:
    - Sender's object id (32-bit)
    - Message size (upper 16-bit) + Request/Event opcode (lower 16-bit)

*/

bool init_wayland(window_t window);
bool end_wayland(window_t window);

#endif
