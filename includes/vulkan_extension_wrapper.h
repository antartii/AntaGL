#ifndef _VULKAN_EXTENSION_WRAPPER_H
#define _VULKAN_EXTENSION_WRAPPER_H

#include <vulkan/vulkan.h>
#include <stdbool.h>

typedef struct vulkan_extensions_functions {
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
} * vulkan_extensions_functions_t;

bool vulkan_init_extensions_functions(VkInstance instance, vulkan_extensions_functions_t vulkan_extensions_functions);

#endif
