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

#ifdef WAYLAND_SURFACE
#include "xdg-shell-client-protocol.h"
#endif

extern const char *wayland_instance_extensions[];

#define WAYLAND_EXTENSIONS_NAMES wayland_instance_extensions
#define WAYLAND_EXTENSIONS_COUNT 2

/*
    ----- INFO -----
    - x and y coordinates are from top left to bottom right
*/

bool init_wayland(window_t window);
bool end_wayland(window_t window);

#endif
