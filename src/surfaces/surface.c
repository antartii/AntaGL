#include "surfaces/surface.h"

const char *surface_instance_extensions[] = {NULL};

bool end_surface_fallback(surface_context_t surface)
{
    return false;
}

bool init_surface_fallback(surface_context_t surface, window_t window)
{
    return false;
}

bool poll_events_surface_fallback(surface_context_t surface)
{
    return false;
}
