#ifndef _ENGINE_H
#define _ENGINE_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "window.h"
#include "vulkan_wrapper.h"
#include "utils.h"

#ifdef WAYLAND_SURFACE
#include "wayland.h"
#endif

#define ENGINE_ERROR_CODE_DEFAULT EXIT_FAILURE
#define ENGINE_NAME "AntaEngine"
#define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)

typedef struct engine
{
    // window related
    window_t window;

    // vulkan related
    // todo store everything vulkan related into a vulkan wrapper struct
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkSurfaceKHR surface;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
    VkSwapchainKHR swapchain;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphic_pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkViewport viewport;
    uint32_t swapchain_images_count;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;

    struct queue_family_indices queue_family_indices;
    VkQueue graphic_queue;
    VkQueue present_queue;

    VkSemaphore present_complete_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence draw_fence;

    uint32_t image_index;

    struct vulkan_extensions_functions vulkan_extensions_functions;
} * engine_t;

void engine_cleanup(engine_t engine);
engine_t engine_create(const char *application_name, const struct version application_version);

#endif