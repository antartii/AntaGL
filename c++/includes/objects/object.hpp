#ifndef _OBJECT_HPP
#define _OBJECT_HPP

#include <AntaGL/AntaGL.h>
#include "../engine.hpp"
#include <vector>
#include <array>

namespace AntaGL {
    class Object {
        public :
            Object(AntaGL::Engine &engine, std::vector<vec2> verticesPos, vec3 color, std::vector<uint16_t> indices);
            Object(object_t object);
            ~Object();

            object_t data() {return _object;}
            void destroy(AntaGL::Engine &engine);

        protected:
            object_t _object;
    };

    class Triangle : public Object {
        public :
            Triangle(AntaGL::Engine &engine, mat3x2 verticlesPos, vec3 color);
            ~Triangle();
    };

    class Rectangle : public Object {
        public :
            Rectangle(AntaGL::Engine &engine, vec2 pos, vec2 size, vec3 color);
            ~Rectangle();
    };

    class Circle : public Object {
        public:
            Circle(AntaGL::Engine &engine, vec2 center, float radius, vec3 color, unsigned int outsideVerticesCount = CIRCLE_DEFAULT_OUTSIDE_VERTICES_COUNT);
            ~Circle();
    };
}

#endif
