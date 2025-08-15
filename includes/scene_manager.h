#ifndef _SCENE_MANAGER_H
#define _SCENE_MANAGER_H

#include "object.h"
#include "engine.h"

#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

// todo more efficient tree because probably too much malloc and bad design ?
typedef struct scene_node {
    struct scene_node *parent;
    struct scene_node **childrens;
    int index;
    int childrens_size;
    object_t object;
} * scene_node_t;

scene_node_t scene_node_create(scene_node_t parent, object_t object);
void scene_node_remove_child(scene_node_t child, scene_node_t node);
void scene_node_destroy(scene_node_t node, bool recursive);
bool scene_node_draw(engine_t engine, scene_node_t node);

#ifdef __cplusplus
}
#endif

#endif
