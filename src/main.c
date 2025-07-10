#include <vulkan/vulkan.h>

#include "utils.h"
#include "vulkan_wrapper.h"
#include "wayland.h"

int main(const int argc, const char **argv, const char **env)
{
    struct window win = {0};

    win.width = 800;
    win.height = 600;

    if (!init_wayland(&win))
        exit(1);

    struct engine engine = {0};
    engine.window = &win;

    init_vulkan(&engine);

    /*while (!win.should_close && wl_display_dispatch(win.display) != -1) {
        
    }*/

    // draw_frame(&engine);
    cleanup(&engine);

    end_wayland(&win);
    return 0;
}
