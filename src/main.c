#include <vulkan/vulkan.h>

#include "utils.h"
#include "vulkan_wrapper.h"

int main(const int argc, const char **argv, const char **env)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;

    struct engine engine = {0};
    engine.name = "AntaEngine";
    engine.version = VK_MAKE_VERSION(1, 0, 0);

    create_vulkan_instance(&engine, app_name, app_version, vulkan_api_version);
    if ((engine.physical_device = pick_physical_device(engine.instance)) == NULL)
        error("Couldn't pick a physical device\n", &engine);
    create_device(&engine);
    create_command_buffers(&engine);
    create_sync_objects(&engine);

    draw_frame(&engine);

    cleanup(&engine);
    return 0;
}
