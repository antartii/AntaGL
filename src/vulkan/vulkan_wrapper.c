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

static void vulkan_get_instance_extensions_names(const char **extensions_names, uint32_t *extensions_count)
{
    if (!extensions_names) {
        *extensions_count = 0;

        #ifdef WAYLAND_SURFACE
        *extensions_count += WAYLAND_EXTENSIONS_COUNT;
        #endif
        #ifdef DEBUG
        (*extensions_count)++;
        #endif
        return;
    }

    int start = 0;
    int offset = 0;

    #ifdef WAYLAND_SURFACE
    while (offset < start + WAYLAND_EXTENSIONS_COUNT)
        extensions_names[offset++] = wayland_instance_extensions[offset - start];
    #endif

    #ifdef DEBUG
    extensions_names[offset++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
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

static void vulkan_get_instance_layers_names(const char **layers_names, uint32_t *layers_count)
{
    if (!layers_names) {
        #ifdef DEBUG
        *layers_count = ENGINE_VALIDATION_LAYERS_COUNT;
        #else
        *layers_count = 0;
        #endif
        return;
    }
    
    #ifdef DEBUG
    for (uint32_t i = 0; i < *layers_count; ++i)
        layers_names[i] = validation_layers[i];
    #endif
}

static bool vulkan_setup_debug_messenger(vulkan_context_t context)
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

    return context->vulkan_extensions_functions.vkCreateDebugUtilsMessengerEXT(context->instance, &debug_messenger_info, NULL, &context->debug_messenger) == VK_SUCCESS;
}

static bool vulkan_create_instance(vulkan_context_t context,
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
    vulkan_get_instance_extensions_names(NULL, &extensions_count);
    const char **extensions_names = malloc(sizeof(char *) * extensions_count);
    vulkan_get_instance_extensions_names(extensions_names, &extensions_count);
    if (!vulkan_check_instance_extensions(extensions_names, extensions_count))
        return false;

    uint32_t layers_count;
    vulkan_get_instance_layers_names(NULL, &layers_count);
    const char **layers_names = malloc(sizeof(char *) * layers_count);
    vulkan_get_instance_layers_names(layers_names, &layers_count);
    
    if (!vulkan_check_instance_layers(layers_names, layers_count)) {
        free(layers_names);
        free(extensions_names);
        return false;
    }

    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extensions_count,
        .ppEnabledExtensionNames = extensions_names,
        .enabledLayerCount = layers_count,
        .ppEnabledLayerNames = layers_names
    };

    VkBool32 result = vkCreateInstance(&instance_info, NULL, &context->instance);
    
    free(extensions_names);
    free(layers_names);

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

static bool vulkan_pick_physical_device(vulkan_context_t context)
{
    uint32_t physical_devices_count;
    vkEnumeratePhysicalDevices(context->instance, &physical_devices_count, NULL);
    if (physical_devices_count == 0)
        return false;
    VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * physical_devices_count);
    int *physical_devices_score  = malloc(sizeof(int) * physical_devices_count);
    vkEnumeratePhysicalDevices(context->instance, &physical_devices_count, physical_devices);

    for (uint32_t i = 0; i < physical_devices_count; ++i)
        physical_devices_score [i] = vulkan_get_physical_device_suitability_score(physical_devices[i]);
    
    uint32_t picked_index = 0;    
    for (uint32_t i = 1; i < physical_devices_count; ++i) {
        if (physical_devices_score [i] > physical_devices_score [picked_index])
            picked_index = i;
    }
    bool is_picked_valid = physical_devices_score[picked_index] > 0;

    if (is_picked_valid)
        context->physical_device = physical_devices[picked_index];

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

static bool vulkan_create_logical_device(vulkan_context_t context)
{
    context->queue_family_indices = vulkan_get_queue_families_indices(context->physical_device, context->surface);
    if (context->queue_family_indices.graphic == UINT32_MAX || context->queue_family_indices.present == UINT32_MAX)
        return false;
    float queue_priority = 1.0f;
    bool is_graphic_also_present = context->queue_family_indices.graphic == context->queue_family_indices.present;
    int queue_count = (is_graphic_also_present) ? 1 : 2;

    VkDeviceQueueCreateInfo device_queue_info[queue_count];

    for (int i = 0; i < queue_count; ++i) {
        device_queue_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_info[i].pNext = NULL;
        device_queue_info[i].queueCount = 1;
        device_queue_info[i].pQueuePriorities = &queue_priority;
        device_queue_info[i].flags = 0;
    }

    device_queue_info[0].queueFamilyIndex = context->queue_family_indices.graphic;
    if (!is_graphic_also_present)
        device_queue_info[1].queueFamilyIndex = context->queue_family_indices.present;

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT physical_device_features_extended = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .extendedDynamicState = true,
        .pNext = NULL
    };

    VkPhysicalDeviceVulkan13Features physical_device_features_13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = true,
        .synchronization2 = true,
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

    VkResult result = vkCreateDevice(context->physical_device, &device_info, NULL, &context->device);

    if (result != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(context->device, context->queue_family_indices.graphic, 0, &context->graphic_queue);
    if (!is_graphic_also_present)
        vkGetDeviceQueue(context->device, context->queue_family_indices.present, 0, &context->present_queue);
    else
        context->present_queue = context->graphic_queue;
    return true;
}

static bool vulkan_create_surface(vulkan_context_t context, window_t window)
{
    #ifdef WAYLAND_SURFACE
    VkWaylandSurfaceCreateInfoKHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = window->surface,
        .display = window->display
    };

    return vkCreateWaylandSurfaceKHR(context->instance, &surface_info, NULL, &context->surface) == VK_SUCCESS;
    #else
    return false;
    #endif
}

static VkSurfaceFormatKHR vulkan_choose_swapchain_surface_format(const VkSurfaceFormatKHR *available_formats, const uint32_t available_formats_count)
{
    for (uint32_t i = 0; i < available_formats_count; ++i) {
        if (available_formats[i].format == VK_FORMAT_R8G8B8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

static bool vulkan_create_swapchain(vulkan_context_t context, window_t window)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, context->surface, &surface_capabilities);

    uint32_t surface_formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &surface_formats_count, NULL);
    VkSurfaceFormatKHR *surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &surface_formats_count, surface_formats);
    context->swapchain_image_format = vulkan_choose_swapchain_surface_format(surface_formats, surface_formats_count).format;

    context->swapchain_extent = vulkan_choose_swap_extent(surface_capabilities, window);

    uint32_t min_image_count_request = (uint32_t) max_int(3u, surface_capabilities.minImageCount);
    min_image_count_request = (surface_capabilities.maxImageCount > 0 && min_image_count_request > surface_capabilities.maxImageCount) ? surface_capabilities.maxImageCount : min_image_count_request;

    uint32_t image_count_request = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count_request > surface_capabilities.maxImageCount)
        image_count_request = surface_capabilities.maxImageCount;

    uint32_t available_present_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &available_present_modes_count, NULL);
    VkPresentModeKHR *available_present_modes = malloc(sizeof(VkPresentModeKHR) * available_present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &available_present_modes_count, available_present_modes);

    VkSwapchainCreateInfoKHR swapchain_info = {0};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.flags = 0;
    swapchain_info.surface = context->surface;
    swapchain_info.minImageCount = min_image_count_request;
    swapchain_info.imageFormat = context->swapchain_image_format;
    swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_info.imageExtent = context->swapchain_extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.preTransform = surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = vulkan_choose_swap_present_mode(available_present_modes, available_present_modes_count);
    swapchain_info.clipped = true;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    uint32_t queue_family_indices_arr[] = {context->queue_family_indices.graphic, context->queue_family_indices.present};

    if (context->queue_family_indices.graphic != context->queue_family_indices.present) {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_family_indices_arr;
    } else  {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = NULL;
    }

    VkResult result = vkCreateSwapchainKHR(context->device, &swapchain_info, NULL, &context->swapchain);

    vkGetSwapchainImagesKHR(context->device, context->swapchain, &context->swapchain_images_count, NULL);
    context->swapchain_images = malloc(sizeof(VkImage) * context->swapchain_images_count);
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &context->swapchain_images_count, context->swapchain_images);

    free(surface_formats);
    free(available_present_modes);

    return result == VK_SUCCESS;
}

static bool vulkan_create_image_view(vulkan_context_t context)
{
    VkImageViewCreateInfo image_view_info = {0};
    image_view_info.pNext = NULL;
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = context->swapchain_image_format;
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

    context->swapchain_image_views = malloc(sizeof(VkImageView) * context->swapchain_images_count);

    for (uint32_t i = 0; i < context->swapchain_images_count; ++i) {
        image_view_info.image = context->swapchain_images[i];
        if (vkCreateImageView(context->device, &image_view_info, NULL, &(context->swapchain_image_views[i])) != VK_SUCCESS)
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

static bool vulkan_create_graphic_pipeline(vulkan_context_t context)
{
    uint32_t code_size;
    char *shader_code = read_file(SHADERS_FILE_PATH, &code_size);

    VkShaderModule shader_module = vulkan_create_shader_module(context->device, shader_code, code_size);
    
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

    uint32_t vertex_binding_descriptions_count;
    vertex_get_binding_description(&vertex_binding_descriptions_count, NULL);
    VkVertexInputBindingDescription vertex_binding_descriptions[vertex_binding_descriptions_count];
    vertex_get_binding_description(&vertex_binding_descriptions_count, vertex_binding_descriptions);

    uint32_t vertex_attribute_descriptions_count;
    vertex_get_attribute_description(&vertex_attribute_descriptions_count, NULL);
    VkVertexInputAttributeDescription vertex_attribute_descriptions[vertex_attribute_descriptions_count];
    vertex_get_attribute_description(&vertex_attribute_descriptions_count, vertex_attribute_descriptions);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = vertex_binding_descriptions_count,
        .pVertexBindingDescriptions = vertex_binding_descriptions,
        .vertexAttributeDescriptionCount = vertex_attribute_descriptions_count,
        .pVertexAttributeDescriptions = vertex_attribute_descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    context->viewport = (VkViewport) {
        .x = 0,
        .y = 0,
        .height = context->swapchain_extent.height,
        .width = context->swapchain_extent.width,
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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

    VkPushConstantRange push_constant_range = {
        .offset = 0,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(struct push_constant)
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .flags = 0,
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &context->descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range
    };

    if (vkCreatePipelineLayout(context->device, &pipeline_layout_info, NULL, &context->pipeline_layout) != VK_SUCCESS) {
        vkDestroyShaderModule(context->device, shader_module, NULL);
        return false;
    }

    VkPipelineRenderingCreateInfo pipeline_rendering_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &context->swapchain_image_format
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
        .layout = context->pipeline_layout,
        .renderPass = NULL,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VkResult result = vkCreateGraphicsPipelines(context->device, NULL, 1, &graphic_pipeline_info, NULL, &context->graphic_pipeline);

    free(shader_code);
    vkDestroyShaderModule(context->device, shader_module, NULL);

    return result == VK_SUCCESS;
}

static bool vulkan_create_command_pool(vulkan_context_t context)
{
    VkCommandPoolCreateInfo command_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context->queue_family_indices.graphic
    };

    return vkCreateCommandPool(context->device, &command_pool_info, NULL, &context->command_pool) == VK_SUCCESS;
}

static bool vulkan_create_command_buffers(vulkan_context_t context)
{
    VkCommandBufferAllocateInfo command_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
        .commandPool = context->command_pool
    };

    context->command_buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    return vkAllocateCommandBuffers(context->device, &command_buffer_info, context->command_buffers) == VK_SUCCESS;
}

static void transition_image_layout(
    uint32_t image_index,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags2 src_access_mask,
    VkAccessFlags2 dst_access_mask,
    VkPipelineStageFlags2 src_stage_mask,
    VkPipelineStageFlags2 dst_stage_mask,
    VkImage *swapchain_images,
    VkCommandBuffer command_buffer)
{
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = NULL,
        .srcAccessMask = src_access_mask,
        .dstAccessMask = dst_access_mask,
        .srcStageMask = src_stage_mask,
        .dstStageMask = dst_stage_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchain_images[image_index],
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = NULL,
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

static void vulkan_record_command_buffer(vulkan_context_t context, object_t *objects, uint32_t objects_count)
{
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pNext = NULL,
        .pInheritanceInfo = NULL,
    };

    vkBeginCommandBuffer(context->command_buffers[context->current_frame], &begin_info);

    transition_image_layout(context->image_index, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, context->swapchain_images, context->command_buffers[context->current_frame]);
    
    VkClearValue clear_color = {
        .color = {
            .float32 = {0.0, 0.0, 0.0, 1.0}
        }
    };
    VkRenderingAttachmentInfo attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = NULL,
        .imageView = context->swapchain_image_views[context->image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_color
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = NULL,
        .flags = 0,
        .renderArea = {
            .offset.x = 0,
            .offset.y = 0,
            .extent = context->swapchain_extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info
    };

    vkCmdBeginRendering(context->command_buffers[context->current_frame], &rendering_info);
    vkCmdBindPipeline(context->command_buffers[context->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphic_pipeline);
    vkCmdSetViewport(context->command_buffers[context->current_frame], 0, 1, &context->viewport);
    VkRect2D scissor = {
        .extent = context->swapchain_extent,
        .offset = {
            .x = 0,
            .y = 0
        }
    };
    vkCmdSetScissor(context->command_buffers[context->current_frame], 0, 1, &scissor);

    vkCmdBindDescriptorSets(context->command_buffers[context->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline_layout, 0, 1, &(context->descriptor_sets[context->current_frame]), 0, NULL);

    for (uint32_t i = 0; i < objects_count; ++i) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(context->command_buffers[context->current_frame], 0, 1, &(objects[i]->vertex_buffer), &offset);
        vkCmdBindIndexBuffer(context->command_buffers[context->current_frame], objects[i]->index_buffer, offset, VK_INDEX_TYPE_UINT16);
        vkCmdPushConstants(context->command_buffers[context->current_frame], context->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct push_constant), &objects[i]->vertex_push_constant);
        vkCmdDrawIndexed(context->command_buffers[context->current_frame], objects[i]->indices_count, 1, 0, 0, 0);
    }

    vkCmdEndRendering(context->command_buffers[context->current_frame]);

    transition_image_layout(context->image_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, context->swapchain_images, context->command_buffers[context->current_frame]);
    vkEndCommandBuffer(context->command_buffers[context->current_frame]);
}

static bool vulkan_create_sync_objects(vulkan_context_t context)
{
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    context->present_complete_semaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->render_finished_semaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->in_fligh_fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(context->device, &semaphore_info, NULL, &(context->present_complete_semaphores[i])) != VK_SUCCESS
        || vkCreateSemaphore(context->device, &semaphore_info, NULL, &(context->render_finished_semaphores[i])) != VK_SUCCESS
        || vkCreateFence(context->device, &fence_info, NULL, (&context->in_fligh_fences[i])) != VK_SUCCESS)
            return false;
        
    }
    return true;
}

static void vulkan_cleanup_swapchain(vulkan_context_t context)
{
    if (context->swapchain_image_views) {
        for (uint32_t i = 0; i < context->swapchain_images_count; ++i)
            vkDestroyImageView(context->device, context->swapchain_image_views[i], NULL);
    }
    free(context->swapchain_image_views);
    free(context->swapchain_images);
    vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
}

void vulkan_recreate_swapchain(vulkan_context_t context, window_t window)
{
    vkDeviceWaitIdle(context->device);
    vulkan_cleanup_swapchain(context);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(context->device, context->present_complete_semaphores[i], NULL);
        vkDestroySemaphore(context->device, context->render_finished_semaphores[i], NULL);
        vkDestroyFence(context->device, context->in_fligh_fences[i], NULL);
    }
    vulkan_create_sync_objects(context);

    vulkan_create_swapchain(context, window);
    vulkan_create_image_view(context);
    context->current_frame = 0;
}

static void vulkan_update_uniform_buffer(vulkan_context_t context, uint32_t current_image)
{
    static struct timespec start_time = {0};

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    if (start_time.tv_sec == 0 && start_time.tv_nsec == 0)
        start_time = current_time;

    double elapsed_sec = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
    float time = (float) elapsed_sec;

    mat4 identity;
    glm_mat4_identity(identity);

    struct uniform_buffer uniform_buffer = {0};
    glm_lookat((vec3) {2.0f, 2.0f, 2.0f}, (vec3) {0.0f, 0.0f, 0.0f}, (vec3) {0.0f, 0.0f, 1.0f}, uniform_buffer.view);

    glm_perspective(glm_rad(45.0f), context->swapchain_extent.width / context->swapchain_extent.height, 1.0f, 10.0f, uniform_buffer.proj);
    uniform_buffer.proj[1][1] *= -1;

    memcpy(context->uniform_buffers_mapped[current_image], &uniform_buffer, sizeof(uniform_buffer));
}

bool vulkan_draw_frame(vulkan_context_t context, window_t window, object_t *objects, uint32_t objects_count)
{
    while (VK_TIMEOUT == vkWaitForFences(context->device, 1, &context->in_fligh_fences[context->current_frame], VK_TRUE, UINT64_MAX));
    VkResult result = vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->present_complete_semaphores[context->current_frame], NULL, &context->image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || window->framebuffer_resized) {
        vulkan_recreate_swapchain(context, window);
        window->framebuffer_resized = false;
        return true;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        write(STDERR_FILENO, "Failed to acquire swapchain image\n", 35);
        return false;
    }
    vkResetFences(context->device, 1, &context->in_fligh_fences[context->current_frame]);

    vkResetCommandBuffer(context->command_buffers[context->current_frame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    vulkan_record_command_buffer(context, objects, objects_count);

    vulkan_update_uniform_buffer(context, context->current_frame);

    VkPipelineStageFlags wait_destination_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(context->present_complete_semaphores[context->current_frame]),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &(context->render_finished_semaphores[context->current_frame]),
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &(context->command_buffers[context->current_frame])
    };

    vkQueueSubmit(context->graphic_queue, 1, &submit_info, context->in_fligh_fences[context->current_frame]);

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(context->render_finished_semaphores[context->current_frame]),
        .swapchainCount = 1,
        .pSwapchains = &context->swapchain,
        .pImageIndices = &context->image_index,
        .pResults = NULL,
    };

    result = vkQueuePresentKHR(context->present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        printf("here\n");
        window->framebuffer_resized = false;
        vulkan_recreate_swapchain(context, window);
    } else if (result != VK_SUCCESS) {
        write(STDERR_FILENO, "Failed to present swapchain image\n", 35);
    }

    context->current_frame = (context->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

static bool vulkan_find_memory_type(vulkan_context_t context, uint32_t type_filter, VkMemoryPropertyFlags properties, uint32_t *memory_type)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context->physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            *memory_type = i;
            return true;
        }
    }

    write(STDERR_FILENO, "failed to find suitable memory type\n", 37);
    return false;
}

static bool vulkan_create_buffer(vulkan_context_t context, VkDeviceSize size, VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags memory_properties, VkBuffer *buffer, VkDeviceMemory *memory)
{
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = size,
        .usage = buffer_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };

    if (vkCreateBuffer(context->device, &buffer_info, NULL, buffer) != VK_SUCCESS)
        return false;
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(context->device, *buffer, &memory_requirements);

    uint32_t memory_type_index;
    if (!vulkan_find_memory_type(context, memory_requirements.memoryTypeBits, memory_properties, &memory_type_index))
        return false;

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index
    };

    if (vkAllocateMemory(context->device, &memory_allocate_info, NULL, memory) != VK_SUCCESS
        || vkBindBufferMemory(context->device, *buffer, *memory, 0) != VK_SUCCESS)
        return false;
    return true;
}

bool vulkan_copy_buffer(vulkan_context_t context, VkBuffer *src_buffer, VkBuffer *dst_buffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandBufferCount = 1,
        .commandPool = context->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
    };

    VkCommandBuffer command_copy_buffer;
    vkAllocateCommandBuffers(context->device, &alloc_info, &command_copy_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_copy_buffer, &begin_info);

    VkBufferCopy copy_region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(command_copy_buffer, *src_buffer, *dst_buffer, 1, &copy_region);
    vkEndCommandBuffer(command_copy_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_copy_buffer
    };

    vkQueueSubmit(context->graphic_queue, 1, &submit_info, NULL);
    vkQueueWaitIdle(context->graphic_queue);
}

bool vulkan_create_index_buffer(vulkan_context_t context, object_t object, uint16_t *indices, uint32_t indices_count)
{
    VkDeviceSize size = sizeof(uint16_t) * indices_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    vulkan_create_buffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_memory);
    
    void *data_staging;
    vkMapMemory(context->device, staging_memory, 0, size, 0, &data_staging);
    memcpy(data_staging, indices, size);
    vkUnmapMemory(context->device, staging_memory);

    vulkan_create_buffer(context, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &object->index_buffer, &object->index_memory);
    vulkan_copy_buffer(context, &staging_buffer, &object->index_buffer, size);

    vkDestroyBuffer(context->device, staging_buffer, NULL);
    vkFreeMemory(context->device, staging_memory, NULL);

    return true;
}

bool vulkan_create_vertex_buffer(vulkan_context_t context, object_t object)
{
    VkDeviceSize size = sizeof(struct vertex) * object->vertices_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    vulkan_create_buffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_memory);

    void *data_staging;
    vkMapMemory(context->device, staging_memory, 0, size, 0, &data_staging);
    memcpy(data_staging, object->vertices, size);
    vkUnmapMemory(context->device, staging_memory);

    vulkan_create_buffer(context, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &object->vertex_buffer, &object->vertex_memory);
    vulkan_copy_buffer(context, &staging_buffer, &object->vertex_buffer, size);
    vkDestroyBuffer(context->device, staging_buffer, NULL);
    vkFreeMemory(context->device, staging_memory, NULL);

    return true;
}

static bool vulkan_create_descriptor_set_layout(vulkan_context_t context)
{
    VkDescriptorSetLayoutBinding descriptor_binding = {
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .descriptorCount = 1,
        .pImmutableSamplers = NULL,
        .binding = 0
    };

    VkDescriptorSetLayoutCreateInfo descriptor_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .bindingCount = 1,
        .pBindings = &descriptor_binding,
        .flags = 0
    };

    if (vkCreateDescriptorSetLayout(context->device, &descriptor_info, NULL, &context->descriptor_set_layout) != VK_SUCCESS)
        return false;
    return true;
}

static bool vulkan_create_descriptor_pool(vulkan_context_t context)
{
    VkDescriptorPoolSize size = {
        .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    };

    VkDescriptorPoolCreateInfo descriptor_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes = &size
    };

    return vkCreateDescriptorPool(context->device, &descriptor_info, NULL, &context->descriptor_pool) == VK_SUCCESS;
}

static bool vulkan_create_uniform_buffers(vulkan_context_t context)
{
    context->uniform_buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    context->uniform_buffers_memory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
    context->uniform_buffers_mapped = malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDeviceSize size = sizeof(struct uniform_buffer);
        VkBuffer buffer;
        VkDeviceMemory buffer_memory;

        if (!vulkan_create_buffer(context, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &(context->uniform_buffers[i]), &(context->uniform_buffers_memory[i]))
            || vkMapMemory(context->device, context->uniform_buffers_memory[i], 0, size, 0, &(context->uniform_buffers_mapped[i])) != VK_SUCCESS)
            return false;
    }
    return true;
}

static bool vulkan_create_descriptor_sets(vulkan_context_t context)
{
    VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);
    context->descriptor_sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        layouts[i] = context->descriptor_set_layout;

    VkDescriptorSetAllocateInfo descriptor_set_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = context->descriptor_pool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    if (vkAllocateDescriptorSets(context->device, &descriptor_set_info, context->descriptor_sets) != VK_SUCCESS) {
        free(layouts);
        return false;
    }
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = context->uniform_buffers[i],
            .offset = 0,
            .range = sizeof(struct uniform_buffer)
        };

        VkWriteDescriptorSet write_descriptor = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = NULL,
            .dstSet = context->descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info
        };

        vkUpdateDescriptorSets(context->device, 1, &write_descriptor, 0, NULL);
    }

    free(layouts);
    return true;
}

bool vulkan_init(vulkan_context_t context,
    window_t window,
    const char *engine_name,
    uint32_t engine_version,
    const char *application_name,
    uint32_t application_version)
{
    return vulkan_create_instance(context, engine_name, engine_version, application_name, application_version)
        && vulkan_init_extensions_functions(context->instance, &context->vulkan_extensions_functions)
        #ifdef DEBUG
        && vulkan_setup_debug_messenger(context)
        #endif
        && vulkan_create_surface(context, window)
        && vulkan_pick_physical_device(context)
        && vulkan_create_logical_device(context)
        && vulkan_create_swapchain(context, window)
        && vulkan_create_image_view(context)
        && vulkan_create_descriptor_set_layout(context)
        && vulkan_create_graphic_pipeline(context)
        && vulkan_create_command_pool(context)
        && vulkan_create_uniform_buffers(context)
        && vulkan_create_descriptor_pool(context)
        && vulkan_create_descriptor_sets(context)
        && vulkan_create_command_buffers(context)
        && vulkan_create_sync_objects(context);
}

void vulkan_cleanup(vulkan_context_t context)
{
    if (context->device) {
        vulkan_cleanup_swapchain(context);

        if (context->descriptor_pool) vkFreeDescriptorSets(context->device, context->descriptor_pool, MAX_FRAMES_IN_FLIGHT, context->descriptor_sets);
        vkDestroyDescriptorPool(context->device, context->descriptor_pool, NULL);

        if (context->uniform_buffers && context->uniform_buffers_mapped) {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                vkDestroyBuffer(context->device, context->uniform_buffers[i], NULL);
                vkFreeMemory(context->device, context->uniform_buffers_memory[i], NULL);
            }
        }

        vkDestroyDescriptorSetLayout(context->device, context->descriptor_set_layout, NULL);

        if (context->present_complete_semaphores) {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                vkDestroySemaphore(context->device, context->present_complete_semaphores[i], NULL);
        }
        if (context->render_finished_semaphores) {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                vkDestroySemaphore(context->device, context->render_finished_semaphores[i], NULL);
        }        
        if (context->in_fligh_fences) {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                vkDestroyFence(context->device, context->in_fligh_fences[i], NULL);
        }

        if (context->command_pool) {
            vkFreeCommandBuffers(context->device, context->command_pool, MAX_FRAMES_IN_FLIGHT, context->command_buffers);
            vkDestroyCommandPool(context->device, context->command_pool, NULL);
        }
        vkDestroyPipeline(context->device, context->graphic_pipeline, NULL);
        vkDestroyPipelineLayout(context->device, context->pipeline_layout, NULL);
        vkDestroyDevice(context->device, NULL);
    }

    free(context->descriptor_sets);
    free(context->uniform_buffers_mapped);
    free(context->uniform_buffers);
    free(context->uniform_buffers_memory);
    free(context->present_complete_semaphores);
    free(context->render_finished_semaphores);
    free(context->in_fligh_fences);

    #ifdef DEBUG
    if (context->vulkan_extensions_functions.vkDestroyDebugUtilsMessengerEXT)
        context->vulkan_extensions_functions.vkDestroyDebugUtilsMessengerEXT(context->instance, context->debug_messenger, NULL);
    #endif

    if (context->instance) vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);
}
