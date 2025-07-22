#include <vulkan/vulkan.h>

#include "utils.h"
#include "engine.h"
#include "model.h"

void run(engine_t engine)
{
    vec2 pos = {-0.5f, -0.5f};
    vec2 size = {1.0f, 1.0f};
    vec3 color = {1.0f, 1.0f, 1.0f};

    model_t rectangle = model_create_rectangle(&engine->vulkan_context, pos, size, color);
    
    mat3x2 tri_pos = {
        {0.0f, -0.5f},
        {0.5f, 0.5f},
        {-0.5f, 0.5f}
    };
    model_t triangle = model_create_triangle(&engine->vulkan_context, tri_pos, color);

    while (!engine->window->should_close) {
        #ifdef WAYLAND_SURFACE
        wl_display_dispatch(engine->window->display);
        #endif

        engine_draw(engine, rectangle);

        if (!engine_display(engine))
            break;
    }

    vkDeviceWaitIdle(engine->vulkan_context.device);
    model_destroy(&engine->vulkan_context, rectangle);
    model_destroy(&engine->vulkan_context, triangle);
}

int main(const int argc, const char **argv, const char **env)
{
    struct version app_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    const char *app_name = "AntaApplication";

    engine_t engine = engine_create(app_name, app_version, 800, 600, 10);

    run(engine);

    engine_cleanup(engine);
    return EXIT_SUCCESS;
}
