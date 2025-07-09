#include "wayland.h"

static void randname(char *buf)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    long r = ts.tv_nsec;
    for (size_t i = 0; i < 6; ++i)
        buf[i] = 'A' + (r & 15) + (r & 16) * 2;
}

static int create_shm_file(void)
{
    int retries = 100;

    do {
        char name[] = "wl_shm_XXXXXX";
        randname(name + sizeof(name) - 7);
        --retries;
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0) {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);

    return -1;
}

int allocate_shm_file(size_t size)
{
    int fd = create_shm_file();
    
    if (fd < 0)
        return -1;

    int ret;

    do {
        ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release
};

static struct wl_buffer *draw_frame(struct window *window)
{
    const int width = 1920;
    const int height = 1080;
    int stride = width * 4;
    int size = stride * height;

    int fd = allocate_shm_file(size);
    if (fd == -1)
        return NULL;
    
    uint32_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(window->shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    // test

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if ((x + y / 8 * 8) % 16 < 8)
                data[y * width + x] = 0xFF666666;
            else
                data[y * width + x] = 0xFFEEEEEE;
        }
    }

    // end test

    munmap(data, size);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    return buffer;
}

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct window *window = data;
    
    xdg_surface_ack_configure(xdg_surface, serial);

    struct wl_buffer *buffer = draw_frame(window);
    
    wl_surface_attach(window->surface, buffer, 0, 0);
    wl_surface_commit(window->surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void registry_handle(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    // printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
    struct window *win = data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        uint32_t compositor_version = version < 4 ? version : 4;
        win->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, compositor_version);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        uint32_t shm_version = version < 1 ? version : 1;
        win->shm = wl_registry_bind(registry, name, &wl_shm_interface, shm_version);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        win->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(win->xdg_wm_base, &xdg_wm_base_listener, win);
    }
}

static void registry_remove(void *data, struct wl_registry *registry, uint32_t name)
{

}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle,
    .global_remove = registry_remove
};

bool init_wayland(struct window *window)
{
    window->display = wl_display_connect(NULL);
    if (!window->display) {
        write(STDERR_FILENO, "Couldn't connect the display\n", 30);
        return false;
    }

    window->registry = wl_display_get_registry(window->display);
    wl_registry_add_listener(window->registry, &registry_listener, window);
    wl_display_roundtrip(window->display);

    window->surface = wl_compositor_create_surface(window->compositor);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->surface);
    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_set_title(window->xdg_toplevel, "Test");
    wl_surface_commit(window->surface);

    return true;
}

bool end_wayland(struct window *window)
{
    if (window->compositor) wl_compositor_destroy(window->compositor);
    if (window->display) wl_registry_destroy(window->registry);
    if (window->surface) wl_surface_destroy(window->surface);
    if (window->shm) wl_shm_destroy(window->shm);
    wl_display_disconnect(window->display);
}
