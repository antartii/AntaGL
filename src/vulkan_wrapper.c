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

    return vkCreateDebugUtilsMessengerEXT_func(instance, &debug_messenger_info, NULL, debug_messenger) == VK_SUCCESS;
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

    return result == VK_SUCCESS;
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

static struct queue_family_indices vulkan_get_queue_families_indices(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    struct queue_family_indices queue_family_indices = {0};

    uint32_t queue_family_properties_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, NULL);
    VkQueueFamilyProperties *queue_family_properties = malloc(sizeof(VkQueueFamilyProperties) * queue_family_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties);

    queue_family_indices.present = UINT32_MAX;
    queue_family_indices.graphic = UINT32_MAX;

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
        VkBool32 graphic_support_present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &graphic_support_present);

        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
            && graphic_support_present) {
            queue_family_indices.graphic = i;
            queue_family_indices.present = i;
            free(queue_family_properties);
            return queue_family_indices;
        }
    }

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queue_family_indices.graphic = i;

        VkBool32 support_present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &support_present);
        if (support_present)
            queue_family_indices.present = i;

        if (queue_family_indices.present != UINT32_MAX
            && queue_family_indices.graphic != UINT32_MAX)
            break;
    }

    free(queue_family_properties);

    return queue_family_indices;
}

bool vulkan_create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice *device, VkQueue *graphic_queue, VkQueue *present_queue)
{
    struct queue_family_indices queue_family_indices = vulkan_get_queue_families_indices(physical_device, surface);
    if (queue_family_indices.graphic == UINT32_MAX || queue_family_indices.present == UINT32_MAX)
        return false;
    float queue_priority = 1.0f;
    bool is_graphic_also_present = queue_family_indices.graphic == queue_family_indices.present;
    int queue_count = (is_graphic_also_present) ? 1 : 2;

    VkDeviceQueueCreateInfo device_queue_info[queue_count];

    for (int i = 0; i < queue_count; ++i) {
        device_queue_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_info[i].pNext = NULL;
        device_queue_info[i].queueCount = 1;
        device_queue_info[i].pQueuePriorities = &queue_priority;
        device_queue_info[i].flags = 0;
    }

    device_queue_info[0].queueFamilyIndex = queue_family_indices.graphic;
    if (!is_graphic_also_present)
        device_queue_info[1].queueFamilyIndex = queue_family_indices.present;

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
        .queueCreateInfoCount = queue_count,
        .pQueueCreateInfos = device_queue_info,
        .enabledExtensionCount = device_extensions_count,
        .ppEnabledExtensionNames = device_extensions
    };

    VkResult result = vkCreateDevice(physical_device, &device_info, NULL, device);

    if (result != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(*device, queue_family_indices.graphic, 0, graphic_queue);
    if (!is_graphic_also_present)
        vkGetDeviceQueue(*device, queue_family_indices.present, 0, present_queue);
    else
        present_queue = graphic_queue;
    return true;
}

bool vulkan_create_surface(VkInstance instance, window_t window, VkSurfaceKHR *surface)
{
    #ifdef WAYLAND_SURFACE
    VkWaylandSurfaceCreateInfoKHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = window->surface,
        .display = window->display
    };

    return vkCreateWaylandSurfaceKHR(instance, &surface_info, NULL, surface) == VK_SUCCESS;
    #else
    return false;
    #endif
}

static VkSurfaceFormatKHR vulkan_choose_swapchain_surface_format(const VkSurfaceFormatKHR *available_formats, const uint32_t available_formats_count)
{
    for (uint32_t i = 0; i < available_formats_count; ++i) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return available_formats[i];
    }
    return available_formats[0];
}

static VkExtent2D vulkan_choose_swap_extent(const VkSurfaceCapabilitiesKHR capabilities, window_t window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    return (VkExtent2D) {
        .width = (uint32_t) clamp_int(window->width, capabilities.minImageExtent.width, capabilities.maxImageExtent.height),
        .height = (uint32_t) clamp_int(window->height, capabilities.minImageExtent.height, capabilities.maxImageExtent.width)
    };
}

static VkPresentModeKHR vulkan_choose_swap_present_mode(const VkPresentModeKHR *available_present_modes, uint32_t available_present_modes_count)
{
    for (uint32_t i = 0; i < available_present_modes_count; ++i) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return available_present_modes[i];
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool vulkan_create_swapchain(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkSurfaceKHR surface,
    window_t window,
    VkSwapchainKHR *swapchain,
    VkFormat *swapchain_image_format,
    VkExtent2D *extent,
    uint32_t *swapchain_images_count,
    VkImage **swapchain_images)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t surface_formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, NULL);
    VkSurfaceFormatKHR *surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, surface_formats);
    *swapchain_image_format = vulkan_choose_swapchain_surface_format(surface_formats, surface_formats_count).format;

    *extent = vulkan_choose_swap_extent(surface_capabilities, window);

    uint32_t min_image_count_request = (uint32_t) max_int(3u, surface_capabilities.minImageCount);
    min_image_count_request = (surface_capabilities.maxImageCount > 0 && min_image_count_request > surface_capabilities.maxImageCount) ? surface_capabilities.maxImageCount : min_image_count_request;

    uint32_t image_count_request = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count_request > surface_capabilities.maxImageCount)
        image_count_request = surface_capabilities.maxImageCount;

    uint32_t available_present_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &available_present_modes_count, NULL);
    VkPresentModeKHR *available_present_modes = malloc(sizeof(VkPresentModeKHR) * available_present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &available_present_modes_count, available_present_modes);

    VkSwapchainCreateInfoKHR swapchain_info = {0};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.flags = 0;
    swapchain_info.surface = surface;
    swapchain_info.minImageCount = min_image_count_request;
    swapchain_info.imageFormat = *swapchain_image_format;
    swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_info.imageExtent = *extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.preTransform = surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = vulkan_choose_swap_present_mode(available_present_modes, available_present_modes_count);
    swapchain_info.clipped = true;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    struct queue_family_indices queue_family_indices = vulkan_get_queue_families_indices(physical_device, surface);
    uint32_t queue_family_indices_arr[] = {queue_family_indices.graphic, queue_family_indices.present};

    if (queue_family_indices.graphic != queue_family_indices.present) {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_family_indices_arr;
    } else  {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = NULL;
    }

    VkResult result = vkCreateSwapchainKHR(device, &swapchain_info, NULL, swapchain);

    vkGetSwapchainImagesKHR(device, *swapchain, swapchain_images_count, NULL);
    *swapchain_images = malloc(sizeof(VkImage) * (*swapchain_images_count));
    vkGetSwapchainImagesKHR(device, *swapchain, swapchain_images_count, *swapchain_images);

    free(surface_formats);
    free(available_present_modes);

    return result == VK_SUCCESS;
}

bool vulkan_create_image_view(VkDevice device, VkFormat swapchain_image_format, uint32_t swapchain_images_count, VkImage *swapchain_images, VkImageView **swapchain_image_views)
{
    VkImageViewCreateInfo image_view_info = {0};
    image_view_info.pNext = NULL;
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = swapchain_image_format;
    image_view_info.subresourceRange = (VkImageSubresourceRange) {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    image_view_info.components = (VkComponentMapping) {
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY
    };

    *swapchain_image_views = malloc(sizeof(VkImageView) * swapchain_images_count);

    for (uint32_t i = 0; i < swapchain_images_count; ++i) {
        image_view_info.image = swapchain_images[i];
        if (vkCreateImageView(device, &image_view_info, NULL, &((*swapchain_image_views)[i])) != VK_SUCCESS)
            return false;
    }

    return true;
}

static VkShaderModule vulkan_create_shader_module(VkDevice device, const char *code, uint32_t code_size)
{
    VkShaderModuleCreateInfo shader_module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = code_size,
        .pCode = (const uint32_t *) code
    };

    VkShaderModule shader_module;

    vkCreateShaderModule(device, &shader_module_info, NULL, &shader_module);

    return shader_module;
}

bool vulkan_create_graphic_pipeline(VkDevice device, VkExtent2D swapchain_extent, VkFormat swapchain_image_format, VkPipelineLayout *pipeline_layout, VkPipeline *graphic_pipeline)
{
    uint32_t code_size;
    const char *shader_code = read_file(SHADERS_FILE_PATH, &code_size);

    VkShaderModule shader_module = vulkan_create_shader_module(device, shader_code, code_size);
    
    VkPipelineShaderStageCreateInfo vert_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = shader_module,
        .pName = SHADER_VERTEX_ENTRY_POINT
    };

    VkPipelineShaderStageCreateInfo frag_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = shader_module,
        .pName = SHADER_FRAGMENT_ENTRY_POINT
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .height = swapchain_extent.height,
        .width = swapchain_extent.width,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkPipelineViewportStateCreateInfo viewport_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterization_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .flags = 0,
        .pNext = NULL,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasClamp = VK_FALSE,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisample_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
        .blendEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .flags = 0,
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, pipeline_layout) != VK_SUCCESS) {
        vkDestroyShaderModule(device, shader_module, NULL);
        return false;
    }

    VkPipelineRenderingCreateInfo pipeline_rendering_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain_image_format
    };

    VkGraphicsPipelineCreateInfo graphic_pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_info,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pViewportState = &viewport_state_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = *pipeline_layout,
        .renderPass = NULL,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VkResult result = vkCreateGraphicsPipelines(device, NULL, 1, &graphic_pipeline_info, NULL, graphic_pipeline);

    vkDestroyShaderModule(device, shader_module, NULL);

    return result == VK_SUCCESS;
}
