#include "wayland.h"

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    struct window *window = data;
    
    window->should_close = true;
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states)
{
    struct window *window = data;

    if (width == 0 || height == 0)
        return;

    window->height = height;
    window->width = width;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .close = xdg_toplevel_handle_close,
    .configure = xdg_toplevel_handle_configure,
    .configure_bounds = NULL,
    .wm_capabilities = NULL
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct window *window = data;

    xdg_surface_ack_configure(xdg_surface, serial);
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
    xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);
    wl_surface_commit(window->surface);

    return true;
}

bool end_wayland(struct window *window)
{
    if (window->compositor) wl_compositor_destroy(window->compositor);
    if (window->display) wl_registry_destroy(window->registry);
    if (window->surface) wl_surface_destroy(window->surface);
    wl_display_disconnect(window->display);
}
