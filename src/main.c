#include <unistd.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "vulkan_wrapper.h"

void cleanup(struct engine *engine)
{
    if (!engine)
        return;

    if (engine->device) vkDestroyDevice(engine->device, NULL);
    if (engine->instance) vkDestroyInstance(engine->instance, NULL);
}

void error(const char *message, struct engine *engine)
{
    cleanup(engine);
    write(STDERR_FILENO, message, strlen(message));
    exit(1);
}

int main(const int argc, const char **argv, const char **env)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    char engine_name[11] = "AntaEngine";
    uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;
    struct engine engine = {
        .instance = NULL,
        .physical_device = NULL,
        .device = NULL
    };

    if (!create_vulkan_instance(&engine.instance, app_name, app_version, "AntaEngine", engine_version, vulkan_api_version))
        error("Couldn't create a vulkan instance\n", &engine);

    if ((engine.physical_device = pick_physical_device(engine.instance)) == NULL)
        error("Couldn't pick a physical device\n", &engine);

    if (!create_device(engine.physical_device, &engine.device))
        error("Couldn't create a logical device\n", &engine);

    cleanup(&engine);
    return 0;
}
