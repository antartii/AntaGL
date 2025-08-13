#ifndef _OBJECT_H
    #define _OBJECT_H

    #include "vertex.h"
    #include "vulkan/shaders.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief typedef to the engine structure needed by some objects functions from "engine.h"
 */
typedef struct engine * engine_t;

/**
 * @struct object
 * @brief Structure representing an object it's properties and buffers
 * @var object::indices_count
 * Count of indices when drawing all the sub triangles composing the model
 * @var object::vertex_push_constant
 * Push constant variable for the vertex shader stage of the model
 * @var object::vertex_buffer
 * Buffer storing all the vertices data
 * @var object::vertex_memory
 * GPU memory storing all the vertices data
 * @var object::index_buffer
 * Buffer storing all the indices data from the sub triangles composing the model
 * @var object::index_memory
 * GPU memory storing all the indices data from the sub triangles composing the model
 */
typedef struct object {
    uint32_t indices_count;

    struct push_constant vertex_push_constant;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_memory;
} * object_t;

/**
 * @brief Create an object and allocate it's buffers and GPU memory
 * 
 * @param engine Pointer to the engine that will create the object
 * @param vertices_pos Pointer to an array of vec2 representing the positions of every vertices
 * @param color Initial color of the created object
 * @param indices Pointer to an array of indices that will create the sub triangles composing the model
 * @param vertices_count Count of vertices in the array `vertices_pos`
 * @return An allocated `struct object` of the object
 */
object_t object_create(engine_t engine, vec2 *vertices_pos, vec3 color, uint16_t *indices, uint32_t vertices_count);
/**
 * @brief Destroy and free all the allocated memory of an object
 * You should call `engine_wait_idle` beforehand to make sure no process are running while you want to destroy the object
 * 
 * @param engine Pointer to the engine that will destroy the object, it should be the same engine that created it
 * @param object Pointer to the object to destroy
 */
void object_destroy(engine_t engine, object_t object);
/**
 * @brief Create a triangle object
 * 
 * @param engine Pointer to the engine that will create the object
 * @param vertices_pos 3x2 matrix of positions for the triangle's vertices
 * @param color Color of the triangle
 * @return The allocated object structure of the triangle
 */
object_t object_create_triangle(engine_t engine, mat3x2 vertices_pos, vec3 color); // todo change pos by mat3 ? to have only 3 vec2 no more
/**
 * @brief Create a rectangle object
 * 
 * @param engine Pointer to the engine that will create the object
 * @param pos Position of the bottom-right corner of the rectangle
 * @param size Size in width and height of the rectangle
 * @param color Color of the rectangle
 * @return object_t 
 */
object_t object_create_rectangle(engine_t engine, vec2 pos, vec2 size, vec3 color);

#ifdef __cplusplus
    }
#endif

#endif
