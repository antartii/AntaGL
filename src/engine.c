#include "engine.h"

static int engine_error(engine_t engine, const char *msg, const bool is_critical)
{
    write(STDERR_FILENO, msg, strlen(msg));

    if (is_critical) {
        engine_cleanup(engine);
        exit(ENGINE_ERROR_CODE_DEFAULT);
    }
    else
        return ENGINE_ERROR_CODE_DEFAULT;
}

void engine_cleanup(engine_t engine)
{
    if (!engine)
        return;

    vulkan_cleanup(&engine->vulkan_context);

    #ifdef WAYLAND_SURFACE
    end_wayland(engine->window);
    #endif

    free(engine->window);
    free(engine);
}

static bool engine_init_window(window_t window)
{
    #ifdef WAYLAND_SURFACE
    return init_wayland(window);
    #else
    return false;
    #endif
}

static void engine_init(engine_t engine, const char *application_name, uint32_t application_version, int window_width, int window_height)
{
    if (!engine)
        engine_error(engine, "engine_init: engine_t engine is NULL\n", true);

    engine->window->width = window_width;
    engine->window->height = window_height;
    engine->window->title = application_name;

    if (!engine_init_window(engine->window))
        engine_error(engine, "engine_init: window couldn't be inited\n", true);

    if (!vulkan_init(&engine->vulkan_context, engine->window, ENGINE_NAME, ENGINE_VERSION, application_name, application_version))
        engine_error(engine, "engine_init: failed to init the engine\n", true);
}

bool engine_draw(engine_t engine, object_t object)
{
    if (engine->objects_to_draw_count >= engine->max_objects_to_draw)
        return false;

    engine->objects_to_draw[engine->objects_to_draw_count++] = object;
    return true;
}

bool engine_display(engine_t engine)
{
    bool result = vulkan_draw_frame(&engine->vulkan_context, engine->window, engine->objects_to_draw, engine->objects_to_draw_count);
    engine->objects_to_draw_count = 0;

    return result;
}

engine_t engine_create(const char *application_name, const struct version application_version, int window_width, int window_height, uint32_t max_objects_to_draw)
{
    engine_t engine = calloc(1, sizeof(struct engine));
    engine->window = calloc(1, sizeof(struct window));
    engine->objects_to_draw = calloc(max_objects_to_draw, sizeof(object_t));
    engine->max_objects_to_draw = max_objects_to_draw;

    if (!engine)
        engine_error(engine, "engine_create: engine_t engine is NULL\n", true);
    engine_init(engine, application_name, VK_MAKE_VERSION(application_version.major, application_version.minor, application_version.patch), window_width, window_height);

    return engine;
}
