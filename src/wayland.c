#include "wayland.h"

bool init_wayland(struct window *window)
{
    window->display = wl_display_connect(NULL);

    if (!window->display) {
        write(STDERR_FILENO, "Couldn't connect the display\n", 30);
        return false;
    }

    return true;
}

bool end_wayland(struct window *window)
{
    wl_display_disconnect(window->display);
}
