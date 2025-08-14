#include <engine.hpp>
#include <objects/object.hpp>

namespace AntaGL {
    Engine::Engine(std::string appName, struct version appVersion, int width, int height, uint32_t maxObjectsToDraw)
    {
        _engine = engine_create(appName.data(), appVersion, width, height, maxObjectsToDraw);
    }

    Engine::~Engine()
    {
        engine_cleanup(_engine);
    }

    bool Engine::display()
    {
        return engine_display(_engine);
    }

    bool Engine::draw(Object object)
    {
        return engine_draw(_engine, object.data());
    }

    bool Engine::pollEvents()
    {
        return engine_poll_events(_engine);
    }

    bool Engine::shouldClose()
    {
        return engine_should_close(_engine);
    }

    void Engine::waitIdle()
    {
        engine_wait_idle(_engine);
    }

    void Engine::updateCamera()
    {
        engine_update_camera(_engine);
    }
}
