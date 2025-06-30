#ifndef _ENGINE_H
#define _ENGINE_H

#include <vulkan/vulkan.h>

struct engine {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
};

#endif
