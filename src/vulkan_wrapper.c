#include "vulkan_wrapper.h"

VkInstance create_vulkan_instance(
    const char *app_name,
    uint32_t app_version,
    const char *engine_name,
    uint32_t engine_version,
    uint32_t higher_vulkan_api_version)
{
    if (higher_vulkan_api_version > get_vulkan_instance_api_version())
        write(STDOUT_FILENO, "Using a lower instance of vulkan api than the higher version requested\n", 72);

    VkInstance instance;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = app_name;
    appInfo.applicationVersion = app_version;
    appInfo.pEngineName = engine_name;
    appInfo.engineVersion = engine_version;
    appInfo.apiVersion = higher_vulkan_api_version;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = NULL;
    createInfo.pApplicationInfo = NULL;
    createInfo.flags = 0;
    createInfo.ppEnabledLayerNames = NULL;
    createInfo.ppEnabledExtensionNames = NULL;
    
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
        return NULL;

    return instance;
}

VkPhysicalDevice *get_physical_devices(VkInstance *vulkan_instance, uint32_t *physical_devices_count)
{
    VkPhysicalDevice *physical_devices = NULL;
    VkResult result;

    vkEnumeratePhysicalDevices(vulkan_instance, &physical_devices_count, NULL);
    result = vkEnumeratePhysicalDevices(vulkan_instance, &physical_devices_count, physical_devices);

    if (result == VK_INCOMPLETE)
        write(STDOUT_FILENO, "The enumeration of physical devices requested is incomplete", 60);
    else if (result != VK_SUCCESS)
        return NULL;
    return physical_devices;
}

VkPhysicalDevice pick_physical_device(VkInstance *vulkan_instance)
{
    uint32_t physical_devices_count = 0;
    VkPhysicalDevice *physical_devices = get_physical_devices(vulkan_instance, &physical_devices_count);

    if (physical_devices_count <= 0 || physical_devices == NULL)
        return NULL;

    uint32_t queue_family_properties_count = 0;
    VkQueueFamilyProperties *queue_family_properties;
    int is_graphic = 0;
    int is_transfer = 0;
    
    for (uint32_t i = 0; i < physical_devices_count; ++i) {
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_devices[i], &queue_family_properties_count, NULL);
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_devices[i], &queue_family_properties_count, queue_family_properties);

        for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
            if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                is_graphic = 1;
            if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                is_transfer = 1;
            if (is_graphic && is_transfer)
                return physical_devices[i];
        }
    }

    return NULL;
}
