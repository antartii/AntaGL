#ifndef _VERTEX_H
#define _VERTEX_H

#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include <stddef.h>

typedef struct vertex {
    vec2 pos;
    vec3 color;
} * vertex_t;

void vertex_get_binding_description(uint32_t *vertex_binding_descriptions_count, VkVertexInputBindingDescription *vertex_binding_descriptions);
void vertex_get_attribute_description(uint32_t *vertex_attribute_descriptions_count, VkVertexInputAttributeDescription *vertex_attribute_descriptions);

#endif
