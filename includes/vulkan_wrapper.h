#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <vulkan/vulkan.h>
#include <unistd.h>
#include <stdbool.h>

// ----- INSTANCE -----
VkInstance create_vulkan_instance(
    const char *app_name,
    uint32_t app_version,
    const char *engine_name,
    uint32_t engine_version,
    uint32_t higher_vulkan_api_version);

// ----- PHYSICAL DEVICES -----
VkPhysicalDevice *get_physical_devices(VkInstance *vulkan_instance, uint32_t *physical_devices_count);
VkPhysicalDevice pick_physical_device(VkInstance *vulkan_instance);

// ----- LOGICAL DEVICES -----
#endif