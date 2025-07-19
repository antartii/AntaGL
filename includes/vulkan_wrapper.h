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

bool vulkan_create_instance(VkInstance *instance,
    const char *engine_name,
    const uint32_t engine_version,
    const char *application_name,
    const uint32_t application_version);

bool vulkan_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger, PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_func);
bool vulkan_pick_physical_device(VkInstance instance, VkPhysicalDevice *physical_device);
bool vulkan_create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice *device, VkQueue *graphic_queue, VkQueue *present_queue, struct queue_family_indices *queue_family_indices);
bool vulkan_create_surface(VkInstance instance, window_t window, VkSurfaceKHR *surface);
bool vulkan_create_swapchain(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkSurfaceKHR surface,
    window_t window,
    struct queue_family_indices queue_family_indices,
    VkSwapchainKHR *swapchain,
    VkFormat *swapchain_image_format,
    VkExtent2D *extent,
    uint32_t *swapchain_images_count,
    VkImage **swapchain_images);

bool vulkan_create_image_view(VkDevice device, VkFormat swapchain_image_format, uint32_t swapchain_images_count, VkImage *swapchain_images, VkImageView **swapchain_image_views);
bool vulkan_create_graphic_pipeline(VkDevice device, VkExtent2D swapchain_extent, VkFormat swapchain_image_format, VkPipelineLayout *pipeline_layout, VkPipeline *graphic_pipeline, VkViewport *viewport);
bool vulkan_create_command_pool(VkDevice device, struct queue_family_indices queue_family_indices, VkCommandPool *command_pool);
bool vulkan_create_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer *command_buffer);
bool vulkan_create_sync_objects(VkDevice device, VkSemaphore *present_complete_semaphore, VkSemaphore *render_finished_semaphore, VkFence *draw_fence);
void vulkan_draw_frame(VkDevice device,
    VkSwapchainKHR *swapchain,
    VkImageView *swapchain_image_views,
    VkExtent2D swapchain_extent,
    VkImage *swapchain_images,
    VkCommandBuffer command_buffer,
    VkPipeline graphic_pipeline, 
    VkViewport viewport,
    VkQueue graphic_queue,
    VkQueue present_queue,
    VkSemaphore *present_complete_semaphore,
    VkSemaphore *render_finished_semaphore,
    VkFence *draw_fence,
    uint32_t *image_index);

#endif
