#ifndef _VULKAN_WRAPPER_H
#define _VULKAN_WRAPPER_H

#include <vulkan/vulkan.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "window.h"

// for wayland
#include "wayland.h"

#define ENGINE_ERROR_CODE_DEFAULT EXIT_FAILURE

struct engine
{
    window_t window;
};

typedef struct engine * engine_t;

void engine_cleanup(engine_t engine);
engine_t engine_create(void);

#endif