#include "vulkan_extension_wrapper.h"

bool vulkan_init_extensions_functions(VkInstance instance, vulkan_extensions_functions_t vulkan_extensions_functions)
{
    vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (!vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT)
        return false;
    return true;
}
