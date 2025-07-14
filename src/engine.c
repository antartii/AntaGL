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

static void vulkan_cleanup(engine_t engine)
{
    if (engine->instance) vkDestroyInstance(engine->instance, NULL);
}

void engine_cleanup(engine_t engine)
{
    if (!engine)
        return;

    vulkan_cleanup(engine);

    end_wayland(engine->window);

    free(engine->window);
    free(engine);
}

static void engine_init(engine_t engine, const char *application_name, uint32_t application_version)
{
    if (!engine)
        engine_error(engine, "engine_init: engine_t engine is NULL\n", true);

    // todo: find a better way to set them manually + do resize
    engine->window->width = 800;
    engine->window->height = 600;

    if (!init_wayland(engine->window))
        engine_error(engine, "engine_init: wayland window couldn't be inited\n", true);
    if (!vulkan_create_instance(&engine->instance, ENGINE_NAME, ENGINE_VERSION, application_name, application_version))
        engine_error(engine, "engine_init: failed to init the engine\n", true);
}

engine_t engine_create(const char *application_name, const struct version application_version)
{
    engine_t engine = calloc(1, sizeof(struct engine));
    engine->window = calloc(1, sizeof(struct window));
    

    if (!engine)
        engine_error(engine, "engine_create: engine_t engine is NULL\n", true);
    engine_init(engine, application_name, VK_MAKE_VERSION(application_version.major, application_version.minor, application_version.patch));

    return engine;
}
