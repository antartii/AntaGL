#ifndef _SCENE_MANAGER_HPP
#define _SCENE_MANAGER_HPP

#include <AntaGL/AntaGL.h>
#include <optional>
#include "objects/object.hpp"
#include "engine.hpp"

namespace AntaGL {
    class SceneNode {
        public:
            SceneNode(Object object);
            SceneNode(SceneNode &parent, Object object);
            ~SceneNode();

            void removeChild(SceneNode child);
            bool draw(Engine &engine);

            scene_node_t data() {return _node;}

        private:
            scene_node_t _node;
    };
}

#endif
