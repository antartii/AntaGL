#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <vulkan/vulkan.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES_IN_FLIGHT 2

// ----- QUEUES -----

/**
 * @brief Structure with simplified informations on queue families
 */
struct queue_family_indices {
    uint32_t graphic; /**< Index for the queue family with graphic properties  */
    uint32_t transfer; /**< Index for the queue family with transfer properties */
};

/**
 * @brief Retrieve in an allocated array the queue family properties from a physical device
 * 
 * @param physical_device Physical device whose properties will be queried
 * @param queue_family_properties_count Pointer to an unsigned integer related to the number of queue family properties contained in the array
 * @return An allocated array of VkQueueFamilyProperties2 or NULL if the request failed
 */
VkQueueFamilyProperties2 *get_queue_family_properties(VkPhysicalDevice physical_device, uint32_t *queue_family_properties_count);
/**
 * @brief Returns the most suitable queue family from an array of queue families
 * 
 * @param queue_family_properties Array of queue families
 * @param queue_family_properties_count Number of queue families contained in queue_family_properties
 * @return A struct queue_family of simplified informations from the most suitable queue family contained in the array
 */

// ----- ENGINE -----
/**
 * @brief Main structure containing the variables used by/for the vulkan wrapper
 * 
 */
struct engine {
    uint32_t version; /**< Developper-supplied version of the engine encoded following the vulkan version's encoding */
    char *name; /**< Developper-supplied name of the engine */

    VkInstance instance; /**< Vulkan instance */
    VkPhysicalDevice physical_device; /**< Physical device */
    VkDevice device; /**< Logical device */
    VkCommandPool command_pool; /**< Command pool for the command buffers */
    uint32_t command_buffer_count; /**< Number of command buffers */
    VkCommandBuffer *command_buffers; /**< Single command buffer allocated (single for now but will be changed later) */
    VkFence *fences;
    VkSemaphore *render_ready_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkQueue graphic_queue; /**< Graphic queue */
    VkQueue present_queue; /**< Present queue */

    uint32_t current_frame; /**< Current frame */

    struct queue_family_indices queue_family_indices; /**< Variables containing the indices of different queue families */
};

/**
 * @brief Destroy, clean and free allocated memory used by the variables inside of an engine structure, then write a message in stderr before exiting the program with code 1. Similar to throw in C++
 * 
 * @param message Pointer to a string that will be written in stderr before quitting
 * @param engine Engine structure to destroy
 */
void error(const char *message, struct engine *engine);

/**
 * @brief Destroy, clean and free the allocated memory used by the variables contained inside of an engine structure.
 * 
 * @param engine Engine structure to destroy
 */
void cleanup(struct engine *engine);

void draw_frame(struct engine *engine);

// ----- INSTANCE -----

/**
 * @brief Create a vulkan instance using the specified values as application's informations
 * 
 * @param engine Pointer to an engine where the vkInstance will be created, the variable .name and .version will also be used for the application's info upon creation
 * @param app_name Pointer to a string containing the application's name
 * @param app_version Unsigned integer of the application's version
 * @param higher_vulkan_api_version Unsigned integer of the higher version of the vukan api the program can run
 */
void create_vulkan_instance(
    struct engine *engine,
    const char *app_name,
    uint32_t app_version,
    uint32_t higher_vulkan_api_version);
/**
 * @brief Retrieve the current version of the vulkan api instance installed on the system
 * 
 * @return An unsigned integer of the version encoded like a VK_API_MAKE_VERSION()
 */
uint32_t get_vulkan_instance_api_version(void);

// ----- PHYSICAL DEVICES -----

/**
 * @brief Retrieve a list of all physical devices installed on the system
 * 
 * @param vulkan_instance Instance of vulkan created beforehand
 * @param physical_devices_count Pointer to an integer representing the number of physical devices requested
 * @return A pointer to the start of an allocated array containing the physical devices installed on the system or NULL if the request fails
 */
VkPhysicalDevice *get_physical_devices(VkInstance vulkan_instance, uint32_t *physical_devices_count);
/**
 * @brief Returns the best VkPhysicalDevice available
 * 
 * @param vulkan_instance Instance of vulkan created beforehand
 * @return A VkPhysicalDevice or NULL if no physical devices are suitable
 */
VkPhysicalDevice pick_physical_device(VkInstance vulkan_instance);

// ----- LOGICAL DEVICES -----

/**
 * @brief Create a logical device for the vulkan API with their queues
 * 
 * @param engine Engine structure containing a vkPhysical device retrieved from vkEnumeratePhysicalDevice() where the vkDevice will be created
 */
void create_device(struct engine *engine);

// ----- COMMAND BUFFERS -----
struct queue_family_indices pick_queue_family(VkQueueFamilyProperties2 *queue_family_properties, uint32_t queue_family_properties_count);
/**
 * @brief Allocate the command buffers and create it's vkCommandPool inside of the engine class
 * 
 * @param engine engine structure where the newly vkCommandBuffer and vkCommandPool will be created
 */
void create_command_buffers(struct engine *engine);

/**
 * @brief Record the specified command buffer and submit it
 * 
 * @param command_buffer Command buffer to submit
 * @param engine Engine structure used for the operations
 */
void record_command_buffer(VkCommandBuffer *command_buffer, struct engine *engine);

// ----- SYNCHRONIZATION AND CACHE CONTROL -----
void create_sync_objects(struct engine *engine);

#endif
