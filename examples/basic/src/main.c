#include <AntaGL/AntaGL.h>
#include <stdio.h>

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
        if (!engine_poll_events(engine)
            || !engine_draw(engine, rectangle)
            || !engine_display(engine))
            break;
    }
    engine_wait_idle(engine);

    object_destroy(engine, rectangle);
    object_destroy(engine, triangle);
}

#ifdef _WIN32
bool redirect_io_to_console()
{
    bool console_is_attached = AttachConsole(ATTACH_PARENT_PROCESS);
    if (!console_is_attached)
        AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    setvbuf(stderr, NULL, _IONBF, 0);
    freopen_s(&fp, "CONIN$", "r", stdin);
    setvbuf(stdin, NULL, _IONBF, 0);

    return console_is_attached;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    bool console_is_attached = redirect_io_to_console();

    struct version app_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    const char *app_name = "AntaApplication";
    engine_t engine = engine_create(app_name, app_version, 800, 600, 10);

    run(engine);

    engine_cleanup(engine);

    if (!console_is_attached)
        FreeConsole();
    return EXIT_SUCCESS;
}
#else
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
#endif
