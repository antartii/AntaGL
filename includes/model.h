#ifndef _MODEL_H
#define _MODEL_H

#include "vertex.h"

typedef struct vulkan_context * vulkan_context_t;

typedef struct model {
    struct vertex *vertices;
    uint32_t vertices_count;
    uint32_t indices_count;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_memory;
} * model_t;

model_t model_create(vulkan_context_t context, vec2 *pos, vec3 color, uint16_t *indices, uint32_t vertices_count);
void model_destroy(vulkan_context_t context, model_t model);

model_t model_create_triangle(vulkan_context_t context, vec2 *pos, vec3 color);
model_t model_create_rectangle(vulkan_context_t context, vec2 *pos, vec3 color);

#endif
