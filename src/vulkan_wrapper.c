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
    bool result = true;
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
            result = false;
    }

    free(extensions_properties);
    return result;
}

static bool vulkan_check_instance_layers(const char **queried_layers, uint32_t layers_count)
{
    bool result = true;
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
            result = false;
    }

    free(layer_properties);
    return result;
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
        .apiVersion = VK_API_VERSION_1_3 // for now maybe changing later
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

    VkBool32 result = vkCreateInstance(&instance_info, NULL, instance);
    
    free(extensions_names);
    #ifdef DEBUG
    free(layers_names);
    #endif

    return !(result != VK_SUCCESS);
}

static int vulkan_get_physical_device_suitability_score(VkPhysicalDevice physical_device)
{
    int score = 0;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    vkGetPhysicalDeviceProperties(physical_device, &properties);
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    if (!features.geometryShader)
        return 0;
    
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    if (properties.apiVersion >= VK_API_VERSION_1_3)
        score += 10000;

    score += properties.limits.maxImageDimension2D;

    return score;
}

static uint32_t vulkan_find_queue_families(VkPhysicalDevice physical_device)
{
    uint32_t queue_families_properties_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_properties_count, NULL);
    VkQueueFamilyProperties *queue_families_properties = malloc(sizeof(VkQueueFamilyProperties) * queue_families_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_properties_count, queue_families_properties);

    for (uint32_t i = 0; i < queue_families_properties_count; ++i) {
        if (queue_families_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            return i;
    }

    return QUEUE_FAMILY_INDICE_DEFAULT;
}

bool vulkan_pick_physical_device(VkInstance instance, VkPhysicalDevice *physical_device)
{
    uint32_t physical_devices_count;
    vkEnumeratePhysicalDevices(instance, &physical_devices_count, NULL);
    if (physical_devices_count == 0)
        return false;
    VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * physical_devices_count);
    int *physical_devices_score  = malloc(sizeof(int) * physical_devices_count);
    vkEnumeratePhysicalDevices(instance, &physical_devices_count, physical_devices);

    for (uint32_t i = 0; i < physical_devices_count; ++i)
        physical_devices_score [i] = vulkan_get_physical_device_suitability_score(physical_devices[i]);
    
    uint32_t picked_index = 0;    
    for (uint32_t i = 1; i < physical_devices_count; ++i) {
        if (physical_devices_score [i] > physical_devices_score [picked_index])
            picked_index = i;
    }
    bool is_picked_valid = physical_devices_score[picked_index] > 0;

    if (is_picked_valid)
        *physical_device = physical_devices[picked_index];

    free(physical_devices);
    free(physical_devices_score);
    return is_picked_valid;
}

static struct queue_family_indices vulkan_get_queue_families_indices(VkPhysicalDevice physical_device)
{
    struct queue_family_indices queue_family_indices = {0};

    uint32_t queue_family_properties_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, NULL);
    VkQueueFamilyProperties *queue_family_properties = malloc(sizeof(VkQueueFamilyProperties) * queue_family_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, NULL);

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_indices.graphic = i;
            break;
        }
    }

    free(queue_family_properties);

    return queue_family_indices;
}

bool vulkan_create_logical_device(VkPhysicalDevice physical_device, VkDevice *device, VkQueue *graphic_queue)
{
    struct queue_family_indices queue_family_indices = vulkan_get_queue_families_indices(physical_device);
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = queue_family_indices.graphic,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT physical_device_features_extended = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .extendedDynamicState = true,
        .pNext = NULL
    };

    VkPhysicalDeviceVulkan13Features physical_device_features_13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = true,
        .pNext = &physical_device_features_extended,
    };

    VkPhysicalDeviceFeatures2 physical_device_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &physical_device_features_13
    };

    const char *device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
    };
    const uint32_t device_extensions_count = 4;

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &physical_device_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_info,
        .enabledExtensionCount = device_extensions_count,
        .ppEnabledExtensionNames = device_extensions
    };

    VkBool32 result = vkCreateDevice(physical_device, &device_info, NULL, device);

    if (result != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(*device, queue_family_indices.graphic, 0, graphic_queue);
    return true;
}
