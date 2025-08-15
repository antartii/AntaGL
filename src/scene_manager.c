#include "scene_manager.h"

static void scene_node_add_child(scene_node_t new_child, scene_node_t dest)
{
    scene_node_t *new_childrens_arr = malloc(sizeof(scene_node_t) * (dest->childrens_size + 1));

    if (dest->childrens != NULL) {
        memcpy(new_childrens_arr, dest->childrens, sizeof(scene_node_t) * dest->childrens_size);
        free(dest->childrens);
    }
    new_childrens_arr[dest->childrens_size++] = new_child;
    dest->childrens = new_childrens_arr;
    new_child->index = dest->childrens_size - 1;
}

scene_node_t scene_node_create(scene_node_t parent, object_t object)
{
    scene_node_t scene_node = calloc(1, sizeof(struct scene_node));

    scene_node->parent = parent;
    scene_node->object = object;

    if (parent != NULL)
        scene_node_add_child(scene_node, parent);
    return scene_node;
}

void scene_node_remove_child(scene_node_t child, scene_node_t node)
{
    if (child->index == -1)
        return;

    if (node->childrens_size == 1) {
        free(node->childrens);
        node->childrens = NULL;
    } else {
        scene_node_t *new_childrens_arr = malloc(sizeof(scene_node_t) * (node->childrens_size - 1));
        int index = 0;

        for (int i = 0; i < node->childrens_size; ++i) {
            if (i != child->index)
                new_childrens_arr[index++] = node->childrens[i];
        }

        if (node->childrens != NULL)
            free(node->childrens);
        node->childrens = new_childrens_arr;
    }

    child->index = 0;
    child->parent = NULL;
    node->childrens_size--;
}

void scene_node_destroy(scene_node_t node, bool recursive)
{
    if (node->parent)
        scene_node_remove_child(node, node->parent);

    for (int i = 0; i < node->childrens_size; ++i) {
        if (recursive)
            scene_node_destroy(node->childrens[i], true);
        else
            scene_node_remove_child(node->childrens[i], node);
    }

    free(node->childrens);
    free(node);
}

bool scene_node_draw(engine_t engine, scene_node_t node)
{
    for (int i = 0; i < node->childrens_size; ++i) {
        if (!scene_node_draw(engine, node->childrens[i]))
            return false;
    }
    return engine_draw(engine, node->object);
}
