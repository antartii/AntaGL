#include "object.h"
#include "engine.h"

object_t object_create(engine_t engine, vec2 *vertices_pos, vec3 color, uint16_t *indices, uint32_t vertices_count)
{
    object_t object = calloc(1, sizeof(struct object));
    struct vertex *vertices = malloc(sizeof(struct vertex) * vertices_count);
    object->indices_count = (vertices_count - 2) * 3;
    glm_mat4_identity(object->vertex_push_constant.model);

    for (uint32_t i = 0; i < vertices_count; ++i) {
        glm_vec3(color, vertices[i].color);
        glm_vec2(vertices_pos[i], vertices[i].pos);
    }

    if (!vulkan_create_vertex_buffer(&engine->vulkan_context, object, vertices, vertices_count)
        || !vulkan_create_index_buffer(&engine->vulkan_context, object, indices, object->indices_count)) {
        free(vertices);
        free(object);
        return NULL;
    }
    free(vertices);

    return object;
}

void object_destroy(engine_t engine, object_t object)
{
    vkDestroyBuffer(engine->vulkan_context.device, object->vertex_buffer, NULL);
    vkFreeMemory(engine->vulkan_context.device, object->vertex_memory, NULL);

    vkDestroyBuffer(engine->vulkan_context.device, object->index_buffer, NULL);
    vkFreeMemory(engine->vulkan_context.device, object->index_memory, NULL);

    free(object);
}

object_t object_create_triangle(engine_t engine, mat3x2 vertices_pos, vec3 color)
{
    uint16_t indices[] = {
        0, 1, 2
    };

    return object_create(engine, vertices_pos, color, indices, 3);
}

object_t object_create_rectangle(engine_t engine, vec2 pos, vec2 size, vec3 color)
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

    return object_create(engine, vertices, color, indices, 4);
}
