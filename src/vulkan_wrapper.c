#include "vulkan_wrapper.h"

uint32_t get_vulkan_instance_api_version(void)
{
    uint32_t api_version = VK_API_VERSION_1_0;

    vkEnumerateInstanceVersion(&api_version);
    return api_version;

}

bool create_vulkan_instance(
    VkInstance *instance,
    const char *app_name,
    uint32_t app_version,
    const char *engine_name,
    uint32_t engine_version,
    uint32_t higher_vulkan_api_version)
{
    if (higher_vulkan_api_version > get_vulkan_instance_api_version())
        write(STDOUT_FILENO, "Using a lower instance of vulkan api than the higher version requested\n", 72);

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = app_version;
    app_info.pEngineName = engine_name;
    app_info.engineVersion = engine_version;
    app_info.apiVersion = higher_vulkan_api_version;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = 0;
    create_info.enabledLayerCount = 0;
    create_info.pNext = NULL;
    create_info.pApplicationInfo = NULL;
    create_info.flags = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.ppEnabledExtensionNames = NULL;
    
    if (vkCreateInstance(&create_info, NULL, instance) != VK_SUCCESS)
        return false;
    return true;
}

VkPhysicalDevice *get_physical_devices(VkInstance vulkan_instance, uint32_t *physical_devices_count)
{
    VkPhysicalDevice *physical_devices = NULL;
    VkResult result;

    vkEnumeratePhysicalDevices(vulkan_instance, physical_devices_count, NULL);
    physical_devices = malloc(sizeof(VkPhysicalDevice) * *physical_devices_count);
    result = vkEnumeratePhysicalDevices(vulkan_instance, physical_devices_count, physical_devices);

    if (result == VK_INCOMPLETE)
        write(STDOUT_FILENO, "The enumeration of physical devices requested is incomplete", 60);
    else if (result != VK_SUCCESS)
        return NULL;
    return physical_devices;
}

VkPhysicalDevice pick_physical_device(VkInstance vulkan_instance)
{
    uint32_t physical_devices_count = 0;
    VkPhysicalDevice *physical_devices = get_physical_devices(vulkan_instance, &physical_devices_count);
    VkPhysicalDevice picked_device = NULL;

    if (physical_devices_count <= 0 || physical_devices == NULL)
        return NULL;

    uint32_t queue_family_properties_count = 0;
    VkQueueFamilyProperties2 *queue_family_properties;
    
    for (uint32_t i = 0; i < physical_devices_count; ++i) {
        queue_family_properties = get_queue_family_properties(physical_devices[i], &queue_family_properties_count);
        struct queue_family queue_family = pick_queue_family(queue_family_properties, queue_family_properties_count);
        free(queue_family_properties);
        if (queue_family.has_graphic && queue_family.has_transfer) {
            picked_device = physical_devices[i];
            break;
        }
    }
    free(physical_devices);
    return picked_device;
}

VkQueueFamilyProperties2 *get_queue_family_properties(VkPhysicalDevice physical_device, uint32_t *queue_family_properties_count)
{
    VkQueueFamilyProperties2 *queue_family_properties;

    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, queue_family_properties_count, NULL);
    queue_family_properties = malloc(sizeof(VkQueueFamilyProperties2) * *queue_family_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, queue_family_properties_count, queue_family_properties);

    return queue_family_properties;
}

struct queue_family pick_queue_family(VkQueueFamilyProperties2 *queue_family_properties, uint32_t queue_family_properties_count)
{
    struct queue_family queue_family = {
        .index = 0,
        .has_graphic = false,
        .has_transfer = false
    };

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
        queue_family.index = i;
        queue_family.has_graphic = queue_family_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        queue_family.has_transfer = queue_family_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT;
        if (queue_family.has_graphic && queue_family.has_transfer)
            break;
    }

    return queue_family;
}

bool create_device(VkPhysicalDevice physical_device, VkDevice *device)
{
    float queue_family_priority = 1.0f;
    uint32_t queue_family_properties_count;
    VkQueueFamilyProperties2 *queue_family_properties = get_queue_family_properties(physical_device, &queue_family_properties_count);
    struct queue_family queue_family = pick_queue_family(queue_family_properties, queue_family_properties_count);
    free(queue_family_properties);

    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = NULL;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = queue_family.index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_family_priority;

    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.ppEnabledExtensionNames = NULL;
    device_create_info.pEnabledFeatures = NULL;

    if (vkCreateDevice(physical_device, &device_create_info, NULL, device) != VK_SUCCESS)
        return false;
    return true;
}
