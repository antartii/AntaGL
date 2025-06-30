#include <unistd.h>
#include <vulkan/vulkan.h>
#include <stdio.h>

#include "utils.h"
#include "vulkan_wrapper.h"

int main(const int argc, const char **argv, const char **env)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    char engine_name[11] = "AntaEngine";
    uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;
    VkInstance vulkan_instance = create_vulkan_instance(app_name, app_version, "AntaEngine", engine_version, vulkan_api_version);

    if (!vulkan_instance) {
        write(STDOUT_FILENO, "Couldn't create a vulkan instance\n", 35);
        return 1;
    }

    VkPhysicalDevice physical_device = pick_physical_device(vulkan_instance);
    if (physical_device == NULL) {
        write(STDOUT_FILENO, "Couldn't pick a physical device\n", 33);
        return 1;
    }

    VkPhysicalDevice physical_device = pick_physical_device(vulkan_instance);

    vkDestroyInstance(vulkan_instance, NULL);
    return 0;
}
