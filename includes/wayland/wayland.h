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

/**
 * @brief Array of the names for the extensions needed by the instance to use wayland
 */
extern const char *wayland_instance_extensions[];

/**
 * @def WAYLAND_EXTENSIONS_NAMES 
 * @brief names for the wayland extensions
 */
#define WAYLAND_EXTENSIONS_NAMES wayland_instance_extensions
/**
 * @def WAYLAND_EXTENSIONS_COUNT
 * @brief Total count of extensions stored in WAYLAND_EXTENSIONS_NAMES
 */
#define WAYLAND_EXTENSIONS_COUNT 2

/**
 * @brief Initialize wayland and a window using wayland
 * 
 * @param window Pointer to the window to update
 * @return true if the initialisation didn't encountered a problem
 * @return false if the initialisation encountered a problem
 */
bool init_wayland(window_t window);
/**
 * @brief End and destroy variables initialised by wayland
 *
 * @param window Pointer to the window where waylands variables will be destroyed
 * @return true if wayland ended without error
 * @return false if wayland encountered and error
 */
bool end_wayland(window_t window);

#endif
