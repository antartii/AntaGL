#include <vulkan/vulkan.h>

#include "utils.h"
#include "engine.h"
#include "wayland.h"

void run(engine_t engine)
{
    // while condition only for wayland
    while (!engine->window->should_close && wl_display_dispatch(engine->window->display)) {
        
    }
}

int main(const int argc, const char **argv, const char **env)
{
    engine_t engine = engine_create();

    run(engine);

    engine_cleanup(engine);
    return EXIT_SUCCESS;
}
