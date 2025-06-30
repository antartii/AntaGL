#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <vulkan/vulkan.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ----- INSTANCE -----
bool create_vulkan_instance(
    VkInstance *instance,
    const char *app_name,
    uint32_t app_version,
    const char *engine_name,
    uint32_t engine_version,
    uint32_t higher_vulkan_api_version);
uint32_t get_vulkan_instance_api_version(void);

// ----- PHYSICAL DEVICES -----
VkPhysicalDevice *get_physical_devices(VkInstance vulkan_instance, uint32_t *physical_devices_count);
VkPhysicalDevice pick_physical_device(VkInstance vulkan_instance);

// ----- LOGICAL DEVICES -----
bool create_device(VkPhysicalDevice physical_device, VkDevice *device);

// ----- QUEUES -----
struct queue_family {
    uint32_t index;
    bool has_graphic;
    bool has_transfer;
};

VkQueueFamilyProperties2 *get_queue_family_properties(VkPhysicalDevice physical_device, uint32_t *queue_family_properties_count);
struct queue_family pick_queue_family(VkQueueFamilyProperties2 *queue_family_properties, uint32_t queue_family_properties_count);

#endif
