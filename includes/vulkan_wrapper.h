#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef WAYLAND_SURFACE
#include "wayland.h"
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <wayland-client.h>
#endif

#include <vulkan/vulkan.h>
#include "utils.h"
#include "vulkan_extension_wrapper.h"

#ifdef DEBUG
#define ENGINE_VALIDATION_LAYERS_COUNT 1

extern const char *validation_layers[];
#endif

#define QUEUE_FAMILY_INDICE_DEFAULT 0
#define SHADERS_FILE_PATH "shaders/slang.spv"
#define SHADER_VERTEX_ENTRY_POINT "vertMain"
#define SHADER_FRAGMENT_ENTRY_POINT "fragMain"

struct queue_family_indices {
    uint32_t graphic;
    uint32_t present;
};

typedef struct vulkan_context {
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
} * vulkan_context_t;

void vulkan_draw_frame(vulkan_context_t vulkan_context);

bool vulkan_init(vulkan_context_t vulkan_context,
    window_t window,
    const char *engine_name,
    uint32_t engine_version,
    const char *application_name,
    uint32_t application_version);

void vulkan_cleanup(vulkan_context_t vulkan_context);

#endif
