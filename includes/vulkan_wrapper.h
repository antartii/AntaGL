#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <vulkan/vulkan.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ----- INSTANCE -----

/**
 * @brief Create a vulkan instance using the specified values as application's informations
 * 
 * @param instance Pointer to a VkInstance that will store the newly created instance, the pointer will remain unchanged if the creation fails
 * @param app_name Pointer to a string containing the application's name
 * @param app_version Unsigned integer of the application's version
 * @param engine_name Pointer to a string containing the engine's name
 * @param engine_version Unsigned integer of the application's version
 * @param higher_vulkan_api_version Unsigned integer of the higher version of the vukan api the program can run
 * @return true upon success
 * @return false upon failure
 */
bool create_vulkan_instance(
    VkInstance *instance,
    const char *app_name,
    uint32_t app_version,
    const char *engine_name,
    uint32_t engine_version,
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
 * @param physical_device One of the suitable VkPhysicalDevice from get_physical_device()
 * @param device Pointer to a VkDevice that will store the newly created VkDevice
 * @return true upon success
 * @return false upon failure
 */
bool create_device(VkPhysicalDevice physical_device, VkDevice *device);

// ----- QUEUES -----

/**
 * @brief Structure with simplified informations on queue families
 */
struct queue_family {
    uint32_t index; /**< Index of the queue family from the array retrieved from get_queue_family_properties() */
    bool has_graphic; /**< Boolean value related to the graphic flag property */
    bool has_transfer; /**< Boolean value related to the transfer flag property */
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
struct queue_family pick_queue_family(VkQueueFamilyProperties2 *queue_family_properties, uint32_t queue_family_properties_count);

#endif
