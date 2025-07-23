#ifndef _SHADERS_H
#define _SHADERS_H

#include <stdalign.h>
#include <cglm/cglm.h>

struct uniform_buffer {
    alignas(16) mat4 view;
    alignas(16) mat4 proj;
};

struct push_constant {
    alignas(16) mat4 model;
};

#endif
