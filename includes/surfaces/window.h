#ifndef _WINDOW_H
    #define _WINDOW_H

    #include <stdbool.h>

    #include "../utils.h"

    /**
     * @def WINDOW_EDGE_MARGIN
     * @brief Define the margin of error for the detection of the window edge 
     */
    #define WINDOW_EDGE_MARGIN 10.f

#ifdef __cplusplus
extern "C" {
#endif

enum mouse_buttons {
    MOUSE_BTN_LEFT = 1 << 0,
    MOUSE_BTN_RIGHT = 1 << 1,
    MOUSE_BTN_MIDDLE = 1 << 2
};

enum mouse_state_bitmask {
    MOUSE_STATE_HOVERING = 1 << 0,
    MOUSE_STATE_MOVING_WINDOW = 1 << 1, // not used for now because we need to track when it's not moving
    MOUSE_STATE_CLICK = 1 << 2
};

/**
 * @struct mouse
 * @brief structure representing the mouse device
 * @var mouse::pos_x
 * Position x of the mouse
 * @var mouse::pos_y
 * Position y of the mouse
 * @var mouse::btn_clicked_bitmask
 * Bitmask representing the state of mouse buttons, using the `enum mouse_buttons`
 * @var mouse::edge_bitmask
 * Bitmask representing on wich edge the mouse is, using the `enum window_edge_bitmask`
 * @var mouse::mouse_state_bitmask
 * Bitmask representing the state of the mouse, using the `enum mouse_state_bitmask`
 */
typedef struct mouse {
    double pos_x;
    double pos_y;
    enum mouse_buttons btn_clicked_bitmask;
    enum mouse_state_bitmask mouse_state_bitmask;

} * mouse_t;

/**
 * @struct window
 * @brief structure representing the window and its properties
 * @var window::should_close
 * Boolean specifying if the window should close or not due to internal events triggered
 * @var window::width
 * Width of the window
 * @var window::height
 * Height of the window
 * @var window::mouse
 * Mouse device
 * @var window::framebuffer_resize
 * Boolean specifying if the framebuffer should be resized
 * @var window::title
 * Title of the window
 */
typedef struct window
{
    bool should_close;
    int width;
    int height;
    struct mouse mouse;
    bool framebuffer_resized;
    const char *title;
} * window_t;

#ifdef __cplusplus
    }
#endif

#endif
