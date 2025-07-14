#include "vulkan_extension_wrapper.h"

bool vulkan_init_extensions_functions(VkInstance instance, vulkan_extensions_functions_t vulkan_extensions_functions)
{
    vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    vulkan_extensions_functions->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (!vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT
        || !vulkan_extensions_functions->vkDestroyDebugUtilsMessengerEXT)
        return false;
    return true;
}
