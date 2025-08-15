#include "sceneManager.hpp"

namespace AntaGL {
    SceneNode::SceneNode(Object object):
        _node(scene_node_create(NULL, object.data()))
    {
    }

    SceneNode::SceneNode(SceneNode &parent, Object object):
        _node(scene_node_create(parent.data(), object.data()))
    {
    }

    SceneNode::~SceneNode()
    {
        scene_node_destroy(_node, false);
    }

    bool SceneNode::draw(Engine &engine)
    {
        return scene_node_draw(engine.data(), _node);
    }

    void SceneNode::removeChild(SceneNode child)
    {
        scene_node_remove_child(child.data(), _node);
    }
}

