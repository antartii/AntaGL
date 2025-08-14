#ifndef _VULKAN_WRAPPER_H
    #define _VULKAN_WRAPPER_H

    #ifndef _WIN32
        #include <unistd.h>
    #endif
    #include "../surfaces/surface.h"
    #include <stdlib.h>
    #include <stdint.h>
    #include <inttypes.h>
    #include <string.h>
    #include <vulkan/vulkan.h>
    #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    #include <cglm/cglm.h>
    #include <stdalign.h>
    #include <time.h>

    #include "../surfaces/window.h"
    #include "../utils.h"
    #include "vulkan_extension_wrapper.h"
    #include "../vertex.h"
    #include "../object.h"
    #include "../camera.h"

#ifdef DEBUG
#define ENGINE_VALIDATION_LAYERS_COUNT 1

extern const char *validation_layers[];
#endif

#define QUEUE_FAMILY_INDICE_DEFAULT 0
#define SHADER_VERTEX_ENTRY_POINT "vertMain"
#define SHADER_FRAGMENT_ENTRY_POINT "fragMain"
#define MAX_FRAMES_IN_FLIGHT 2

#ifdef _WIN32
    #define SHADER_FILE_PATH "C:/Program Files (x86)/AntaGL/share/AntaGL/shaders/slang.spv"
#else
    #define SHADER_FILE_PATH "/usr/local/share/AntaGL/shaders/slang.spv"
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
    VkCommandBuffer *command_buffers;
    VkViewport viewport;
    uint32_t swapchain_images_count;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *descriptor_sets;

    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    void **uniform_buffers_mapped;

    struct queue_family_indices queue_family_indices;
    VkQueue graphic_queue;
    VkQueue present_queue;

    VkSemaphore *present_complete_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_fligh_fences;
    uint32_t current_frame;

    uint32_t image_index;

    struct vulkan_extensions_functions vulkan_extensions_functions;
} * vulkan_context_t;

bool vulkan_draw_frame(vulkan_context_t vulkan_context, window_t window, object_t *objects, uint32_t objects_count);

bool vulkan_init(vulkan_context_t vulkan_context,
    surface_context_t surface_context,
    window_t window,
    const char *engine_name,
    uint32_t engine_version,
    const char *application_name,
    uint32_t application_version);

void vulkan_cleanup(vulkan_context_t vulkan_context);
bool vulkan_create_vertex_buffer(vulkan_context_t context, object_t object, struct vertex *vertices, uint32_t vertices_count);
bool vulkan_create_index_buffer(vulkan_context_t context, object_t object, uint16_t *indices, uint32_t indices_count);

void vulkan_update_proj(vulkan_context_t context, camera_t camera);
void vulkan_update_view(vulkan_context_t context, camera_t camera);

#ifdef __cplusplus
    }
#endif

#endif
