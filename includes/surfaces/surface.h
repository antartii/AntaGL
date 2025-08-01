#ifndef _SURFACE_H
#define _SURFACE_H

#include "window.h"

#ifdef WAYLAND_SURFACE
    #include "wayland/wayland.h"
    typedef struct wayland_context surface_context;
    typedef wayland_context_t surface_context_t;

    #define end_surface end_wayland
    #define init_surface init_wayland
    #define poll_events_surface poll_events_wayland
    #define SURFACE_EXTENSIONS_COUNT 2
    #define SURFACE_EXTENSIONS_NAMES wayland_instance_extensions
#else
    typedef struct surface {
        int test;
    } surface_context;
    typedef surface_context * surface_context_t;

    bool end_surface_fallback(surface_context_t surface);
    bool init_surface_fallback(surface_context_t surface, window_t window);
    bool poll_events_surface_fallback(surface_context_t surface);

    #define end_surface end_surface_fallback
    #define init_surface init_surface_fallback
    #define poll_events_surface poll_events_surface_fallback

    #define SURFACE_EXTENSIONS_COUNT 0
    extern const char *surface_instance_extensions[];
    #define SURFACE_EXTENSIONS_NAMES surface_instance_extensions
#endif

#endif