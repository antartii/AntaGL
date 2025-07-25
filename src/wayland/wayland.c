#include "wayland/wayland.h"

/*
    ----- INFO -----
    - x and y coordinates are from top left to bottom right make sure to update code that use coordinate accordingly
*/

const char *wayland_instance_extensions[] = {
    "VK_KHR_surface",
    "VK_KHR_wayland_surface"
};

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    window_t window = data;
    
    window->should_close = true;
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states)
{
    window_t window = data;

    if (width == 0 || height == 0)
        return;

    if (window->width != width || window->height != height) {
        window->width = width;
        window->height = height;
        window->framebuffer_resized = true;
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .close = xdg_toplevel_handle_close,
    .configure = xdg_toplevel_handle_configure,
    .configure_bounds = NULL,
    .wm_capabilities = NULL
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    window_t window = data;

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

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    window_t window = data;

    window->mouse.mouse_state_bitmask |= MOUSE_STATE_HOVERING;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
    window_t window = data;

    window->mouse.mouse_state_bitmask &= MOUSE_STATE_HOVERING;
}

static enum window_edge_bitmask mouse_get_edge(double x, double y, int width, int height)
{
    enum window_edge_bitmask window_edge_bitmask = NONE;

    if (x < WINDOW_EDGE_MARGIN) window_edge_bitmask |= WINDOW_EDGE_LEFT;
    else if (x > width - WINDOW_EDGE_MARGIN) window_edge_bitmask |= WINDOW_EDGE_RIGHT;

    if (y < WINDOW_EDGE_MARGIN) window_edge_bitmask |= WINDOW_EDGE_TOP;
    else if (y > height - WINDOW_EDGE_MARGIN) window_edge_bitmask |= WINDOW_EDGE_BOTTOM;

    return window_edge_bitmask;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    window_t window = data;

    //window->state_bitmask |= MOUSE_STATE_MOVING;
    double y = wl_fixed_to_double(surface_y);
    double x = wl_fixed_to_double(surface_x);

    window->mouse.pos_x = x;
    window->mouse.pos_y = window->width - y;
    window->mouse.edge_bitmask = mouse_get_edge(x, y, window->width, window->height);
}

static int mouse_button_code_to_bitmask_value(uint32_t button)
{
    switch (button) {
        case BTN_LEFT: return MOUSE_BTN_LEFT;
        case BTN_RIGHT: return MOUSE_BTN_RIGHT;
        case BTN_MIDDLE: return MOUSE_BTN_MIDDLE;
        default: return 0;
    }
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    window_t window = data;
    uint32_t button_bitmask = mouse_button_code_to_bitmask_value(button);

    if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
        window->mouse.mouse_state_bitmask &= MOUSE_STATE_CLICK;
        window->mouse.btn_clicked_bitmask &= button_bitmask;
    } else {
        window->mouse.mouse_state_bitmask |= MOUSE_STATE_CLICK;
        window->mouse.btn_clicked_bitmask |= button_bitmask;
        if (window->mouse.edge_bitmask != NONE)
            xdg_toplevel_resize(window->xdg_toplevel, window->seat, serial, window->mouse.edge_bitmask);
    }
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{
}

static void wl_pointer_axis_relative_direction(void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction)
{
}

static void wl_pointer_axis_value_120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120)
{
}

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .frame = wl_pointer_frame,
    .axis = wl_pointer_axis,
    .axis_stop = wl_pointer_axis_stop,
    .axis_source = wl_pointer_axis_source,
    .axis_discrete = wl_pointer_axis_discrete,
    .axis_relative_direction = wl_pointer_axis_relative_direction,
    .axis_value120 = wl_pointer_axis_value_120
};

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
    window_t window = data;

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && !window->pointer) {
        window->pointer = wl_seat_get_pointer(window->seat);
        wl_pointer_add_listener(window->pointer, &wl_pointer_listener, window);
    } else if (!have_pointer && window->pointer) {
        wl_pointer_release(window->pointer);
        window->pointer = NULL;
    }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    // fprintf(stderr, "seat name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name
};

static void zxdg_toplevel_decoration_v1_configure(void *data, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode)
{
    if (mode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
        #ifdef DEBUG
        write(STDERR_FILENO, "Compositor doesn't support server side decoration\n", 51);
        #endif
        return;
    }
}

static const struct zxdg_toplevel_decoration_v1_listener zxdg_toplevel_decoration_v1_listener = {
    .configure = zxdg_toplevel_decoration_v1_configure
};

static void registry_handle(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    // printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
    window_t win = data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        uint32_t compositor_version = version < 4 ? version : 4;
        win->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, compositor_version);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        win->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(win->xdg_wm_base, &xdg_wm_base_listener, win);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        win->seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
        wl_seat_add_listener(win->seat, &wl_seat_listener, win);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        // win->zxdg_decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void registry_remove(void *data, struct wl_registry *registry, uint32_t name)
{

}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle,
    .global_remove = registry_remove
};

bool init_wayland(window_t window)
{
    window->display = wl_display_connect(NULL);
    if (!window->display) {
        #ifdef DEBUG
        write(STDERR_FILENO, "Couldn't connect the display\n", 30);
        #endif
        return false;
    }

    window->registry = wl_display_get_registry(window->display);
    wl_registry_add_listener(window->registry, &registry_listener, window);
    wl_display_roundtrip(window->display);

    window->surface = wl_compositor_create_surface(window->compositor);
    wl_surface_set_input_region(window->surface, NULL);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->surface);
    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_set_title(window->xdg_toplevel, window->title);
    xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);
    
    /*if (window->zxdg_decoration_manager) {
        window->zxdg_toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(window->zxdg_decoration_manager, window->xdg_toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->zxdg_toplevel_decoration, &zxdg_toplevel_decoration_v1_listener, window);
    }*/
    wl_surface_commit(window->surface);

    return true;
}

bool end_wayland(window_t window)
{
    if (window->compositor) wl_compositor_destroy(window->compositor);
    if (window->display) wl_registry_destroy(window->registry);
    if (window->surface) wl_surface_destroy(window->surface);
    wl_display_disconnect(window->display);
}
