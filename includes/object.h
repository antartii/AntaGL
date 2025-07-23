#ifndef _OBJECT_H
#define _OBJECT_H

#include "vertex.h"
#include "shaders.h"

typedef struct vulkan_context * vulkan_context_t;

typedef struct object {
    struct vertex *vertices;
    uint32_t vertices_count;
    uint32_t indices_count;

    struct push_constant vertex_push_constant;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_memory;
} * object_t;

object_t object_create(vulkan_context_t context, vec2 *vertices_pos, vec3 color, uint16_t *indices, uint32_t vertices_count);
void object_destroy(vulkan_context_t context, object_t object);

object_t object_create_triangle(vulkan_context_t context, mat3x2 vertices_pos, vec3 color); // todo change pos by mat3 ? to have only 3 vec2 no more
object_t object_create_rectangle(vulkan_context_t context, vec2 pos, vec2 size, vec3 color);

#endif
