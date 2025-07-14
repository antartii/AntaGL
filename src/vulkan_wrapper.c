#include "vulkan_wrapper.h"

#ifdef DEBUG
const char *validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        fprintf(stderr, "Validation layer: type %" PRIu32 " msg: %s\n", type, pCallbackData->pMessage);

    return VK_FALSE;
}

static void vulkan_get_instance_extensions_names(const char ***extensions_names, uint32_t *extensions_count)
{
    *extensions_count = 0;

    #ifdef WAYLAND_SURFACE
    *extensions_count += WAYLAND_EXTENSIONS_COUNT;
    #endif
    #ifdef DEBUG
    (*extensions_count)++;
    #endif

    *extensions_names = malloc(sizeof(const char *) * (*extensions_count));
    int start = 0;
    int offset = 0;

    #ifdef WAYLAND_SURFACE
    while (offset < start + WAYLAND_EXTENSIONS_COUNT)
        (*extensions_names)[offset++] = wayland_instance_extensions[offset - start];
    #endif

    #ifdef DEBUG
    (*extensions_names)[offset++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    #endif
}

static bool vulkan_check_instance_extensions(const char **queried_extensions_names, uint32_t extensions_count)
{
    uint32_t extensions_properties_count;

    vkEnumerateInstanceExtensionProperties(NULL, &extensions_properties_count, NULL);
    VkExtensionProperties *extensions_properties = malloc(sizeof(VkExtensionProperties) * extensions_properties_count);
    vkEnumerateInstanceExtensionProperties(NULL, &extensions_properties_count, extensions_properties);

    for (uint32_t i = 0; i < extensions_count; ++i) {
        bool extension_found = false;
        for (uint32_t j = 0; j < extensions_properties_count; ++j) {
            if (strcmp(queried_extensions_names[i], extensions_properties[j].extensionName) == 0) {
                extension_found = true;
                break;
            }
        }
        if (!extension_found)
            return false;
    }

    return true;
}

static bool vulkan_check_instance_layers(const char **queried_layers, uint32_t layers_count)
{
    uint32_t layers_properties_count;
    vkEnumerateInstanceLayerProperties(&layers_properties_count, NULL);
    VkLayerProperties *layer_properties = malloc(sizeof(VkLayerProperties) * layers_properties_count);
    vkEnumerateInstanceLayerProperties(&layers_properties_count, layer_properties);

    for (uint32_t i = 0; i < layers_count; ++i) {
        bool layer_found = false;
        for (uint32_t j = 0; j < layers_properties_count; ++j) {
            if (strcmp(queried_layers[i], layer_properties[j].layerName) == 0) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
            return false;
    }

    return true;
}

static void vulkan_get_instance_layers_names(const char ***layers_names, uint32_t *layers_count)
{
    #ifdef DEBUG
    *layers_count = ENGINE_VALIDATION_LAYERS_COUNT;
    *layers_names = malloc(sizeof(const char *) * (*layers_count));

    for (uint32_t i = 0; i < *layers_count; ++i)
        (*layers_names)[i] = validation_layers[i];
    #else
    *layers_count = 0;
    *layers_names = NULL;
    #endif
}

bool vulkan_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger, PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_func)
{
    VkDebugUtilsMessageSeverityFlagsEXT severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = vulkan_debug_callback
    };

    if (vkCreateDebugUtilsMessengerEXT_func(instance, &debug_messenger_info, NULL, debug_messenger) != VK_SUCCESS)
        return false;
    else
        return true;
}

bool vulkan_create_instance(VkInstance *instance,
    const char *engine_name,
    const uint32_t engine_version,
    const char *application_name,
    const uint32_t application_version)
{
    VkApplicationInfo app_info =  {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pEngineName = engine_name,
        .engineVersion = engine_version,
        .pApplicationName = application_name,
        .applicationVersion = application_version,
        .apiVersion = VK_API_VERSION_1_0 // for now maybe changing later
    };

    uint32_t extensions_count;
    const char **extensions_names = NULL;
    vulkan_get_instance_extensions_names(&extensions_names, &extensions_count);
    if (!vulkan_check_instance_extensions(extensions_names, extensions_count))
        return false;

    uint32_t layers_count;
    const char **layers_names = NULL;
    vulkan_get_instance_layers_names(&layers_names, &layers_count);
    if (!vulkan_check_instance_layers(layers_names, layers_count))
        return false;

    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extensions_count,
        .ppEnabledExtensionNames = extensions_names,
        .enabledLayerCount = layers_count,
        .ppEnabledLayerNames = layers_names
    };

    if (vkCreateInstance(&instance_info, NULL, instance) != VK_SUCCESS)
        return false;
    else
        return true;
}

