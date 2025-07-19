#include <vulkan/vulkan.h>

#include "utils.h"
#include "engine.h"
#include "wayland.h"

void run(engine_t engine)
{
    while (!engine->window->should_close) {
        #ifdef WAYLAND_SURFACE
        wl_display_dispatch(engine->window->display);
        #endif

        vulkan_draw_frame(&engine->vulkan_context);
    }

    vkDeviceWaitIdle(engine->vulkan_context.device);   
}

int main(const int argc, const char **argv, const char **env)
{
    struct version app_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    const char *app_name = "AntaApplication";

    engine_t engine = engine_create(app_name, app_version, 800, 600);

    run(engine);

    engine_cleanup(engine);
    return EXIT_SUCCESS;
}
