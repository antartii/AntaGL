#include "objects/object.hpp"

namespace AntaGL {
    Object::Object(AntaGL::Engine &engine, std::vector<vec2> verticesPos, vec3 color, std::vector<uint16_t> indices)
    {
        _object = object_create(engine.data(), verticesPos.data(), color, indices.data(), verticesPos.size());
    }

    Object::Object(object_t object):
        _object(object)
    {
    }

    Object::~Object()
    {
    }

    void Object::destroy(AntaGL::Engine &engine)
    {
        object_destroy(engine.data(), _object);
    }

    // === TRIANGLES ===
    Triangle::Triangle(AntaGL::Engine &engine, mat3x2 verticlesPos, vec3 color):
        Object(object_create_triangle(engine.data(), verticlesPos, color))
    {
    }

    Triangle::~Triangle()
    {

    }

    // === RECTANGLEs ===
    Rectangle::Rectangle(AntaGL::Engine &engine, vec2 pos, vec2 size, vec3 color):
        Object(object_create_rectangle(engine.data(), pos, size, color))
    {
    }

    Rectangle::~Rectangle()
    {

    }
}
