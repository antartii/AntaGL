#include "vulkan_wrapper.h"

void error(const char *message, struct engine *engine)
{
    cleanup(engine);
    write(STDERR_FILENO, message, strlen(message));
    exit(1);
}

void create_surface(struct engine *engine)
{
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo= {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = engine->window->display,
        .surface = engine->window->surface,
        .pNext = NULL,
        .flags = 0
    };

    if (vkCreateWaylandSurfaceKHR(engine->instance, &surfaceCreateInfo, NULL, &engine->surface) != VK_SUCCESS)
        error("Couldn't create wayland surface", engine);
}

void draw_frame(struct engine *engine)
{
    vkWaitForFences(engine->device, 1, &engine->fences[engine->current_frame], VK_TRUE, UINT64_MAX);

    vkResetFences(engine->device, 1, &engine->fences[engine->current_frame]);
    vkResetCommandBuffer(engine->command_buffers[engine->current_frame], 0);
    record_command_buffer(&engine->command_buffers[engine->current_frame], engine);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &engine->command_buffers[engine->current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &engine->render_finished_semaphores[engine->current_frame];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.pWaitSemaphores = &engine->image_available_semaphores[engine->current_frame];
    submit_info.waitSemaphoreCount = 1;

    if (vkQueueSubmit(engine->graphic_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
        write(STDERR_FILENO, "Couldn't submit the command buffers\n", 37);

    engine->current_frame = (engine->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void record_command_buffer(VkCommandBuffer *command_buffer, struct engine *engine)
{
    VkCommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = NULL;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(*command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
        write(STDERR_FILENO, "Couldn't start the command buffer recording\n", 45);
        return;
    }

    VkRenderingInfo rendering_info = {0};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = NULL;
    rendering_info.flags = 0;
    rendering_info.renderArea.extent.height = engine->window->height;
    rendering_info.renderArea.extent.width = engine->window->width;
    rendering_info.renderArea.offset.x = 0;
    rendering_info.renderArea.offset.y = 0;

    // vkCmdBeginRendering(*command_buffer, );

    if (vkEndCommandBuffer(*command_buffer) != VK_SUCCESS) {
        write(STDERR_FILENO, "Couldn't submit command buffer\n", 32);
        return;
    }
}

void cleanup(struct engine *engine)
{
    if (!engine)
        return;

    if (engine->device) {
        vkDeviceWaitIdle(engine->device);
        if (engine->command_pool) {
            if (engine->command_buffers) {
                vkFreeCommandBuffers(engine->device, engine->command_pool, engine->command_buffer_count, engine->command_buffers);
                free (engine->command_buffers);
            }
            vkDestroyCommandPool(engine->device, engine->command_pool, NULL);
        }
        if (engine->fences) {
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                vkDestroyFence(engine->device, engine->fences[i], NULL);
            free(engine->fences);
        }
        vkDestroyDevice(engine->device, NULL);
    }
    if (engine->instance) {
        if (engine->surface) vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
        vkDestroyInstance(engine->instance, NULL);
    }
}

uint32_t get_vulkan_instance_api_version(void)
{
    uint32_t api_version = VK_API_VERSION_1_0;

    vkEnumerateInstanceVersion(&api_version);
    return api_version;

}

void create_vulkan_instance(
    struct engine *engine,
    const char *app_name,
    uint32_t app_version,
    uint32_t higher_vulkan_api_version)
{
    if (higher_vulkan_api_version > get_vulkan_instance_api_version())
        write(STDERR_FILENO, "Using a lower instance of vulkan api than the higher version requested\n", 72);

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = app_version;
    app_info.pEngineName = engine->name;
    app_info.engineVersion = engine->version;
    app_info.apiVersion = higher_vulkan_api_version;

    const char *extensions[] = {
        // for wayland
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
    };

    const int extensions_count = 2;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extensions_count;
    create_info.enabledLayerCount = 0;
    create_info.pNext = NULL;
    create_info.pApplicationInfo = &app_info;
    create_info.flags = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.ppEnabledExtensionNames = extensions;
    
    if (vkCreateInstance(&create_info, NULL, &engine->instance) != VK_SUCCESS)
        error("Couldn't create a vulkan instance\n", engine);
}

VkPhysicalDevice *get_physical_devices(VkInstance vulkan_instance, uint32_t *physical_devices_count)
{
    VkPhysicalDevice *physical_devices = NULL;
    VkResult result;

    vkEnumeratePhysicalDevices(vulkan_instance, physical_devices_count, NULL);
    physical_devices = malloc(sizeof(VkPhysicalDevice) * *physical_devices_count);
    result = vkEnumeratePhysicalDevices(vulkan_instance, physical_devices_count, physical_devices);

    if (result == VK_INCOMPLETE)
        write(STDERR_FILENO, "The enumeration of physical devices requested is incomplete", 60);
    else if (result != VK_SUCCESS)
        return NULL;
    return physical_devices;
}

void create_sync_objects(struct engine *engine)
{
    VkSemaphoreCreateInfo semaphore_create_info = {0};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;

    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fence_create_info.pNext = NULL;

    engine->fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    engine->render_finished_semaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    engine->image_available_semaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(engine->device, &semaphore_create_info, NULL, &engine->render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(engine->device, &semaphore_create_info, NULL, &engine->image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(engine->device, &fence_create_info, NULL, &engine->fences[i]) != VK_SUCCESS)
            error("Couldn't create synchronysations objects\n", engine);
    }
}

VkPhysicalDevice pick_physical_device(struct engine *engine)
{
    uint32_t physical_devices_count = 0;
    VkPhysicalDevice *physical_devices = get_physical_devices(engine->instance, &physical_devices_count);
    VkPhysicalDevice picked_device = NULL;

    if (physical_devices_count <= 0 || physical_devices == NULL)
        return NULL;

    uint32_t queue_family_properties_count = 0;
    VkQueueFamilyProperties2 *queue_family_properties;
    
    for (uint32_t i = 0; i < physical_devices_count; ++i) {
        queue_family_properties = get_queue_family_properties(physical_devices[i], &queue_family_properties_count);
        struct queue_family_indices queue_family_indices = pick_queue_family(queue_family_properties, queue_family_properties_count, physical_devices[i], engine);
        free(queue_family_properties);
        
        if (queue_family_indices.has_graphic || queue_family_indices.has_transfer || queue_family_indices.has_present) {
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
    for (uint32_t i = 0; i < *queue_family_properties_count; ++i) {
        queue_family_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        queue_family_properties[i].pNext = NULL;
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, queue_family_properties_count, queue_family_properties);

    return queue_family_properties;
}

struct queue_family_indices pick_queue_family(VkQueueFamilyProperties2 *queue_family_properties, uint32_t queue_family_properties_count, VkPhysicalDevice physical_device, struct engine *engine)
{
    struct queue_family_indices queue_family_indices = {
        .graphic = 0,
        .transfer = 0,
        .present = 0,

        .has_graphic = false,
        .has_present = false,
        .has_present = false
    };

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
        VkQueueFlags flags = queue_family_properties[i].queueFamilyProperties.queueFlags;

        if ((flags & VK_QUEUE_GRAPHICS_BIT) && !queue_family_indices.has_graphic) {
            queue_family_indices.graphic = i;
            queue_family_indices.has_graphic = true;
        }
        if ((flags & VK_QUEUE_TRANSFER_BIT) && !queue_family_indices.has_transfer) {
            queue_family_indices.transfer = i;
            queue_family_indices.has_transfer = true;
        }
        // for wayland

        if (vkGetPhysicalDeviceWaylandPresentationSupportKHR(physical_device, i, engine->window->display) != VK_SUCCESS && !queue_family_indices.has_present) {
            queue_family_indices.present = i;
            queue_family_indices.has_present = true;
        }
    }

    return queue_family_indices;
}

void create_command_buffers(struct engine *engine)
{
    uint32_t queue_family_properties_count;

    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = engine->queue_family_indices.graphic;

    if (vkCreateCommandPool(engine->device, &command_pool_create_info, NULL, &engine->command_pool) != VK_SUCCESS)
        error("Couldn't create command pool\n", engine);

    VkCommandBufferAllocateInfo command_buffer_info;
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext = NULL;
    command_buffer_info.commandPool = engine->command_pool;
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.commandBufferCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    engine->command_buffer_count = MAX_FRAMES_IN_FLIGHT;
    engine->command_buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateCommandBuffers(engine->device, &command_buffer_info, engine->command_buffers) != VK_SUCCESS)
        error("Couldn't allocate the command buffer\n", engine);
}

void create_device(struct engine *engine)
{
    float queue_family_priority = 1.0f;
    uint32_t queue_family_properties_count;
    VkQueueFamilyProperties2 *queue_family_properties = get_queue_family_properties(engine->physical_device, &queue_family_properties_count);
    engine->queue_family_indices = pick_queue_family(queue_family_properties, queue_family_properties_count, engine->physical_device, engine);
    free(queue_family_properties);

    VkDeviceQueueCreateInfo queue_create_infos[2];
    uint32_t queue_create_info_count = 0;

    // graphic queue
    queue_create_infos[queue_create_info_count++] = (VkDeviceQueueCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = engine->queue_family_indices.graphic,
        .queueCount = 1,
        .pQueuePriorities = &queue_family_priority
    };

    // present
    if (engine->queue_family_indices.present != engine->queue_family_indices.graphic) {
        queue_create_infos[queue_create_info_count++] = (VkDeviceQueueCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = engine->queue_family_indices.present,
            .queueCount = 1,
            .pQueuePriorities = &queue_family_priority,
        };
    }

    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = queue_create_info_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.ppEnabledExtensionNames = NULL;
    device_create_info.pEnabledFeatures = NULL;

    if (vkCreateDevice(engine->physical_device, &device_create_info, NULL, &engine->device) != VK_SUCCESS)
        error("Couldn't create a logical device\n", engine);
    /*
    VkDeviceQueueInfo2 graphic_queue_info;
    graphic_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
    graphic_queue_info.pNext = NULL;
    graphic_queue_info.flags = 0;
    graphic_queue_info.queueIndex = 0;
    graphic_queue_info.queueFamilyIndex = queue_family_indices.graphic;
    */

    vkGetDeviceQueue(engine->device, engine->queue_family_indices.graphic, 0, &engine->graphic_queue);
    if (engine->queue_family_indices.present == engine->queue_family_indices.graphic)
        engine->present_queue = engine->graphic_queue;
    else
        vkGetDeviceQueue(engine->device, engine->queue_family_indices.present, 0, &engine->present_queue);
}

void create_render_pass(struct engine *engine)
{
}

void init_vulkan(struct engine *engine)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;

    engine->name = "AntaEngine";
    engine->version = VK_MAKE_VERSION(1, 0, 0);

    create_vulkan_instance(engine, app_name, app_version, vulkan_api_version);
    create_surface(engine);
    if ((engine->physical_device = pick_physical_device(engine)) == NULL)
        error("Couldn't pick a physical device\n", engine);
    create_device(engine);
    create_command_buffers(engine);
    create_sync_objects(engine);
}
