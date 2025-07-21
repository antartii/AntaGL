#include "vertex.h"

void vertex_get_binding_description(uint32_t *vertex_binding_descriptions_count, VkVertexInputBindingDescription *vertex_binding_descriptions)
{
    if (!vertex_binding_descriptions) {
        *vertex_binding_descriptions_count = 1;
        return;
    }
    
    vertex_binding_descriptions[0] = (VkVertexInputBindingDescription) {
        .binding = 0,
        .stride = sizeof(struct vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

void vertex_get_attribute_description(uint32_t *vertex_attribute_descriptions_count, VkVertexInputAttributeDescription *vertex_attribute_descriptions)
{
    if (!vertex_attribute_descriptions) {
        *vertex_attribute_descriptions_count = 2;
        return;
    }

    vertex_attribute_descriptions[0] = (VkVertexInputAttributeDescription) {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(struct vertex, pos)
    };

    vertex_attribute_descriptions[1] = (VkVertexInputAttributeDescription) {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(struct vertex, color)
    };
}
