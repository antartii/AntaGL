#include "vulkan_wrapper.h"

static void vulkan_get_instance_extensions_names(const char ***extensions_names, uint32_t *extensions_count)
{
    *extensions_count = 0;

    #ifdef WAYLAND_SURFACE
    *extensions_count += WAYLAND_EXTENSIONS_COUNT;
    #endif

    *extensions_names = malloc(sizeof(const char *) * (*extensions_count));
    int start = 0;
    int offset = 0;

    #ifdef WAYLAND_SURFACE
    while (offset < start + WAYLAND_EXTENSIONS_COUNT) {
        (*extensions_names)[offset] = wayland_instance_extensions[offset - start];
        offset++;
    }
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

    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extensions_count,
        .ppEnabledExtensionNames = extensions_names
    };

    if (vkCreateInstance(&instance_info, NULL, instance) != VK_SUCCESS)
        return false;
    else
        return true;
}
