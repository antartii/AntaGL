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

bool vulkan_create_instance(VkInstance *instance,
    const char *engine_name,
    const uint32_t engine_version,
    const char *application_name,
    const uint32_t application_version);

bool vulkan_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger, PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_func);

#endif
