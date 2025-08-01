#include <AntaGL/AntaGL.h>

void run(engine_t engine)
{
    vec2 pos = {-1.0f, -1.0f};
    vec2 size = {1.0f, 1.0f};
    vec3 color = {1.0f, 1.0f, 1.0f};

    object_t rectangle = object_create_rectangle(engine, pos, size, color);
    
    mat3x2 tri_pos = {
        {0.0f, -0.5f},
        {0.5f, 0.5f},
        {-0.5f, 0.5f}
    };
    object_t triangle = object_create_triangle(engine, tri_pos, color);

    while (!engine_should_close(engine)) {
        engine_poll_events(engine);

        engine_draw(engine, rectangle);

        if (!engine_display(engine))
            break;
    }
    engine_wait_idle(engine);

    object_destroy(engine, rectangle);
    object_destroy(engine, triangle);
}

int main(const int argc, const char **argv, const char **env)
{
    struct version app_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    const char *app_name = "AntaApplication";

    engine_t engine = engine_create(app_name, app_version, 800, 600, 10);

    run(engine);

    engine_cleanup(engine);
    return EXIT_SUCCESS;
}
