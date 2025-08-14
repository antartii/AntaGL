#ifndef _VERTEX_H
    #define _VERTEX_H

    #include <vulkan/vulkan.h>
    #include <cglm/cglm.h>
    #include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct vertex
 * @brief Structure representing a vertex
 * @var vertex::pos
 * Position of the vertex in a 2D space
 * @var vertex::color
 * RGB color of the vertex from 0.0f to 1.0f
 */
typedef struct vertex {
    vec2 pos;
    vec3 color;
} * vertex_t;

/**
 * @brief Getter for the input binding descriptions of the vertex structure
 * If `vertex_binding_descriptions` is `NULL` returns the total number of input binding descriptions in `vertex_binding_descriptions_count`.
 * Otherwise populate the allocated array `vertex_binding_descriptions`
 * 
 * @param vertex_binding_descriptions_count Pointer to an unsigned int where the total count of input binding descriptions will be stored
 * @param vertex_binding_descriptions Pointer to an allocated array of `vertex_binding_descriptions_count` * sizeof(VkVertexInputBindingDescription) where the input binding descriptions will be stored
 */
void vertex_get_binding_description(uint32_t *vertex_binding_descriptions_count, VkVertexInputBindingDescription *vertex_binding_descriptions);

 /**
 * @brief Getter for the input attribute descriptions of the vertex structure
 * If `vertex_attribute_descriptions` is `NULL` returns the total number of input binding descriptions in `vertex_attribute_descriptions_count`.
 * Otherwise populate the allocated array `vertex_attribute_descriptions`
 * 
 * @param vertex_attribute_descriptions_count Pointer to an unsigned int where the total count of input attribute descriptions will be stored
 * @param vertex_attribute_descriptions Pointer to an allocated array of `vertex_attribute_descriptions_count` * sizeof(VkVertexInputAttributeDescription) where the input attribute descriptions will be stored
 */
void vertex_get_attribute_description(uint32_t *vertex_attribute_descriptions_count, VkVertexInputAttributeDescription *vertex_attribute_descriptions);

#ifdef __cplusplus
    }
#endif

#endif
