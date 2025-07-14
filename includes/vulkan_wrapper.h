#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <unistd.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef WAYLAND_SURFACE
#include "wayland.h"
#endif

#include "utils.h"
#include "vulkan_extension_wrapper.h"

#ifdef DEBUG
#define ENGINE_VALIDATION_LAYERS_COUNT 1

extern const char *validation_layers[];
#endif

#define QUEUE_FAMILY_INDICE_DEFAULT 0

struct queue_family_indices {
    uint32_t graphic;
};

bool vulkan_create_instance(VkInstance *instance,
    const char *engine_name,
    const uint32_t engine_version,
    const char *application_name,
    const uint32_t application_version);

bool vulkan_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger, PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_func);
bool vulkan_pick_physical_device(VkInstance instance, VkPhysicalDevice *physical_device);
bool vulkan_create_logical_device(VkPhysicalDevice physical_device, VkDevice *device, VkQueue *graphic_queue);

#endif
