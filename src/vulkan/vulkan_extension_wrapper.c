#include "vulkan/vulkan_extension_wrapper.h"

bool vulkan_init_extensions_functions(VkInstance instance, vulkan_extensions_functions_t vulkan_extensions_functions)
{
    vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    vulkan_extensions_functions->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    #ifdef DEBUG
    if (!vulkan_extensions_functions->vkCreateDebugUtilsMessengerEXT
        || !vulkan_extensions_functions->vkDestroyDebugUtilsMessengerEXT)
        return false;
    #endif
    return true;
}
