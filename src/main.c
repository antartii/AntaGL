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

        vulkan_draw_frame(
            engine->device,
            &engine->swapchain,
            engine->swapchain_image_views,
            engine->swapchain_extent,
            engine->swapchain_images,
            engine->command_buffer,
            engine->graphic_pipeline,
            engine->viewport,
            engine->graphic_queue,
            engine->present_queue,
            &engine->present_complete_semaphore, 
            &engine->render_finished_semaphore,
            &engine->draw_fence,
            &engine->image_index);
    }

    vkDeviceWaitIdle(engine->device);   
}

int main(const int argc, const char **argv, const char **env)
{
    struct version app_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    const char *app_name = "AntaApplication";

    engine_t engine = engine_create(app_name, app_version);

    run(engine);

    engine_cleanup(engine);
    return EXIT_SUCCESS;
}
