#include "surfaces/wayland/wayland.h"

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
    wayland_context_t context = data;

    context->user_window->should_close = true;
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states)
{
    wayland_context_t context = data;

    if (width == 0 || height == 0)
        return;

    if (context->user_window->width != width || context->user_window->height != height) {
        context->user_window->width = width;
        context->user_window->height = height;
        context->user_window->framebuffer_resized = true;
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
    wayland_context_t context = (wayland_context_t) data;

    context->user_window->mouse.mouse_state_bitmask |= MOUSE_STATE_HOVERING;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
    wayland_context_t context = (wayland_context_t) data;

    context->user_window->mouse.mouse_state_bitmask &= MOUSE_STATE_HOVERING;
}

static enum xdg_toplevel_resize_edge window_get_edge(double x, double y, int width, int height)
{
    enum xdg_toplevel_resize_edge window_edge_bitmask = NONE;

    if (x < WINDOW_EDGE_MARGIN) window_edge_bitmask |= XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    else if (x > width - WINDOW_EDGE_MARGIN) window_edge_bitmask |= XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;

    if (y < WINDOW_EDGE_MARGIN) window_edge_bitmask |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (y > height - WINDOW_EDGE_MARGIN) window_edge_bitmask |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;

    return window_edge_bitmask;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    wayland_context_t context = data;

    //window->state_bitmask |= MOUSE_STATE_MOVING;
    double y = wl_fixed_to_double(surface_y);
    double x = wl_fixed_to_double(surface_x);

    context->user_window->mouse.pos_x = x;
    context->user_window->mouse.pos_y = context->user_window->width - y;
    context->resize_edge_bitmask = window_get_edge(x, y, context->user_window->width, context->user_window->height);
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
    wayland_context_t context = data;
    uint32_t button_bitmask = mouse_button_code_to_bitmask_value(button);

    if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
        context->user_window->mouse.mouse_state_bitmask &= MOUSE_STATE_CLICK;
        context->user_window->mouse.btn_clicked_bitmask &= button_bitmask;
    } else {
        context->user_window->mouse.mouse_state_bitmask |= MOUSE_STATE_CLICK;
        context->user_window->mouse.btn_clicked_bitmask |= button_bitmask;
        if (context->resize_edge_bitmask != NONE)
            xdg_toplevel_resize(context->xdg_toplevel, context->seat, serial, context->resize_edge_bitmask);
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
    wayland_context_t context = data;

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && !context->pointer) {
        context->pointer = wl_seat_get_pointer(context->seat);
        wl_pointer_add_listener(context->pointer, &wl_pointer_listener, context);
    } else if (!have_pointer && context->pointer) {
        wl_pointer_release(context->pointer);
        context->pointer = NULL;
    }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    // fprintf(STDERR_FILENO, "seat name: %s\n", name);
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
    wayland_context_t win = data;

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

bool init_wayland(wayland_context_t context, window_t window)
{
    context->user_window = window;

    context->display = wl_display_connect(NULL);
    if (!context->display) {
        #ifdef DEBUG
        write(STDERR_FILENO, "Couldn't connect the display\n", 30);
        #endif
        return false;
    }

    context->registry = wl_display_get_registry(context->display);
    wl_registry_add_listener(context->registry, &registry_listener, context);
    wl_display_roundtrip(context->display);

    context->surface = wl_compositor_create_surface(context->compositor);
    wl_surface_set_input_region(context->surface, NULL);
    context->xdg_surface = xdg_wm_base_get_xdg_surface(context->xdg_wm_base, context->surface);
    xdg_surface_add_listener(context->xdg_surface, &xdg_surface_listener, context);
    context->xdg_toplevel = xdg_surface_get_toplevel(context->xdg_surface);
    xdg_toplevel_set_title(context->xdg_toplevel, window->title);
    xdg_toplevel_add_listener(context->xdg_toplevel, &xdg_toplevel_listener, context);
    
    /*if (context->zxdg_decoration_manager) {
        context->zxdg_toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(context->zxdg_decoration_manager, context->xdg_toplevel);
        zxdg_toplevel_decoration_v1_add_listener(context->zxdg_toplevel_decoration, &zxdg_toplevel_decoration_v1_listener, context);
    }*/
    wl_surface_commit(context->surface);

    return true;
}

bool end_wayland(wayland_context_t context)
{
    if (context->compositor) wl_compositor_destroy(context->compositor);
    if (context->display) wl_registry_destroy(context->registry);
    if (context->surface) wl_surface_destroy(context->surface);
    wl_display_disconnect(context->display);
}

bool poll_events_wayland(wayland_context_t context)
{
    wl_display_dispatch(context->display);
    return true;
}
