#include <vulkan/vulkan.h>

#include "utils.h"
#include "vulkan_wrapper.h"

int main(const int argc, const char **argv, const char **env)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;

    struct engine engine = {
        .name = "AntaEngine",
        .version = VK_MAKE_VERSION(1, 0, 0),

        .instance = NULL,
        .physical_device = NULL,
        .device = NULL,
        .command_pool = NULL,
        .command_buffer_count = 1,
        .command_buffers = NULL,
        .current_frame = 0,

        .queue_family_indices = {
            .graphic = -1,
            .transfer = -1
        }
    };

    create_vulkan_instance(&engine, app_name, app_version, vulkan_api_version);
    if ((engine.physical_device = pick_physical_device(engine.instance)) == NULL)
        error("Couldn't pick a physical device\n", &engine);
    create_device(&engine);
    create_command_buffers(&engine);

    record_command_buffer(&engine.command_buffers[engine.current_frame], &engine);

    cleanup(&engine);
    return 0;
}
