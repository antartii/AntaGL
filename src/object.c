#include "object.h"
#include "vulkan_wrapper.h"

#include <unistd.h>

object_t object_create(vulkan_context_t context, vec2 *vertices_pos, vec3 color, uint16_t *indices, uint32_t vertices_count)
{
    object_t object = calloc(1, sizeof(struct object));
    object->vertices_count = vertices_count;
    object->vertices = malloc(sizeof(struct vertex) * vertices_count);
    object->indices_count = (vertices_count - 2) * 3;
    glm_mat4_identity(object->vertex_push_constant.model);

    for (uint32_t i = 0; i < vertices_count; ++i) {
        glm_vec3(color, object->vertices[i].color);
        glm_vec2(vertices_pos[i], object->vertices[i].pos);
    }

    if (!vulkan_create_vertex_buffer(context, object)
        || !vulkan_create_index_buffer(context, object, indices, object->indices_count)) {
        free(object->vertices);
        free(object);
        return NULL;
    }

    return object;
}

void object_destroy(vulkan_context_t context, object_t object)
{
    vkDestroyBuffer(context->device, object->vertex_buffer, NULL);
    vkFreeMemory(context->device, object->vertex_memory, NULL);

    vkDestroyBuffer(context->device, object->index_buffer, NULL);
    vkFreeMemory(context->device, object->index_memory, NULL);

    free(object->vertices);
    free(object);
}

object_t object_create_triangle(vulkan_context_t context, mat3x2 vertices_pos, vec3 color)
{
    uint16_t indices[] = {
        0, 1, 2
    };

    return object_create(context, vertices_pos, color, indices, 3);
}

object_t object_create_rectangle(vulkan_context_t context, vec2 pos, vec2 size, vec3 color)
{
    uint16_t indices[] = {
        0, 1, 2, 2, 3, 0
    };

    vec2 opposite_vertice;
    glm_vec2_add(pos, size, opposite_vertice);

    mat4x2 vertices = {
        {pos[0], pos[1]},
        {opposite_vertice[0], pos[1]},
        {opposite_vertice[0], opposite_vertice[1]},
        {pos[0], opposite_vertice[1]}
    };

    return object_create(context, vertices, color, indices, 4);
}
