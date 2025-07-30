#include "engine.h"

static int engine_error(engine_t engine, const char *msg, const bool is_critical)
{
    #ifdef DEBUG
    write(STDERR_FILENO, msg, (unsigned int) strlen(msg));
    #endif

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

    end_surface(&engine->surface_context);

    free(engine->window);
    free(engine);
}

static bool engine_init_window(window_t window, surface_context_t surface)
{
    return init_surface(surface, window);
}

static void engine_init(engine_t engine, const char *application_name, uint32_t application_version, int window_width, int window_height)
{
    if (!engine)
        engine_error(engine, "engine_init: engine_t engine is NULL\n", true);

    engine->window->width = window_width;
    engine->window->height = window_height;
    engine->window->title = application_name;

    if (!engine_init_window(engine->window, &engine->surface_context))
        engine_error(engine, "engine_init: window couldn't be inited\n", true);

    if (!vulkan_init(&engine->vulkan_context, &engine->surface_context, engine->window, ENGINE_NAME, ENGINE_VERSION, application_name, application_version))
        engine_error(engine, "engine_init: failed to init vulkan\n", true);
}

bool engine_draw(engine_t engine, object_t object)
{
    if (engine->objects_to_draw_count >= engine->max_objects_to_draw) {
        #ifdef DEBUG
        write(STDERR_FILENO, "Cannot draw more objects\n", 26);
        #endif
        return false;
    }

    engine->objects_to_draw[engine->objects_to_draw_count++] = object;
    return true;
}

void engine_wait_idle(engine_t engine)
{
    vkDeviceWaitIdle(engine->vulkan_context.device);
}

void engine_poll_events(engine_t engine)
{
    poll_events_surface(&engine->surface_context);
}

bool engine_should_close(engine_t engine)
{
    return engine->window->should_close;
}

bool engine_display(engine_t engine)
{
    bool result = vulkan_draw_frame(&engine->vulkan_context, engine->window, engine->objects_to_draw, engine->objects_to_draw_count);
    engine->objects_to_draw_count = 0;

    return result;
}

void engine_update_camera(engine_t engine)
{
    vulkan_update_proj(&engine->vulkan_context, &engine->camera);
    vulkan_update_view(&engine->vulkan_context, &engine->camera);
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
    camera_init(&engine->camera);
    engine_update_camera(engine);

    return engine;
}
