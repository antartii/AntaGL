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

    end_wayland(engine->window);
}

static void engine_init(engine_t engine)
{
    if (!engine)
        engine_error(engine, "engine_init: engine_t engine is NULL\n", true);

    // todo: find a better way to set them manually + do resize
    engine->window->width = 800;
    engine->window->height = 600;

    if (!init_wayland(engine->window))
        engine_error(engine, "engine_init: wayland window couldn't be inited\n", true);
}

engine_t engine_create(void)
{
    engine_t engine = calloc(1, sizeof(struct engine));
    engine->window = calloc(1, sizeof(struct window));

    if (!engine)
        engine_error(engine, "engine_create: engine_t engine is NULL\n", true);
    engine_init(engine);

    return engine;
}
