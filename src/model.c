#include "model.h"
#include "vulkan_wrapper.h"

#include <unistd.h>

model_t model_create(vulkan_context_t context, vec2 *pos, vec3 color, uint16_t *indices, uint32_t vertices_count)
{
    model_t model = calloc(1, sizeof(struct model));
    model->vertices_count = vertices_count;
    model->vertices = malloc(sizeof(struct vertex) * vertices_count);
    model->indices_count = (vertices_count - 2) * 3;

    for (uint32_t i = 0; i < vertices_count; ++i) {
        glm_vec3(color, model->vertices[i].color);
        glm_vec2(pos[i], model->vertices[i].pos);
    }

    if (!vulkan_create_vertex_buffer(context, model)
        || !vulkan_create_index_buffer(context, model, indices, model->indices_count)) {
        free(model->vertices);
        free(model);
        return NULL;
    }

    return model;
}

void model_destroy(vulkan_context_t context, model_t model)
{
    vkDestroyBuffer(context->device, model->vertex_buffer, NULL);
    vkFreeMemory(context->device, model->vertex_memory, NULL);

    vkDestroyBuffer(context->device, model->index_buffer, NULL);
    vkFreeMemory(context->device, model->index_memory, NULL);

    free(model->vertices);
    free(model);
}

model_t model_create_triangle(vulkan_context_t context, vec2 *pos, vec3 color)
{
    uint16_t indices[] = {
        0, 1, 2
    };

    return model_create(context, pos, color, indices, 3);
}

model_t model_create_rectangle(vulkan_context_t context, vec2 *pos, vec3 color)
{
    uint16_t indices[] = {
        0, 1, 2, 2, 3, 0
    };

    return model_create(context, pos, color, indices, 4);
}
