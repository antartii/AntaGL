#ifndef _ENGINE_HPP
#define _ENGINE_HPP

#include <AntaGL/AntaGL.h>

#include <iostream>
#include <string>

namespace AntaGL {
    class Object;

    class Engine {
        public :
            Engine(std::string appName, struct version appVersion, int width, int height, uint32_t maxObjectsToDraw);
            ~Engine();

            bool display();
            bool draw(Object object);
            bool pollEvents();
            bool shouldClose();
            void waitIdle();
            void updateCamera();

            engine_t data() {return _engine;}

        private:
            engine_t _engine;
    };
}

#endif
