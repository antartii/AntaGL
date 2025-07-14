#ifndef _ENGINE_H
#define _ENGINE_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "window.h"
#include "vulkan_wrapper.h"
#include "utils.h"

#ifdef WAYLAND_SURFACE
#include "wayland.h"
#endif

#define ENGINE_ERROR_CODE_DEFAULT EXIT_FAILURE
#define ENGINE_NAME "AntaEngine"
#define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)

struct engine
{
    window_t window;

    VkInstance instance;
};

typedef struct engine * engine_t;

void engine_cleanup(engine_t engine);
engine_t engine_create(const char *application_name, const struct version application_version);

#endif