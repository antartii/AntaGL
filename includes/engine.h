#ifndef _ENGINE_H
    #define _ENGINE_H

    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdbool.h>

    #include "window.h"
    #include "vulkan/vulkan_wrapper.h"
    #include "utils.h"
    #include "camera.h"

    #ifdef WAYLAND_SURFACE
    #include "wayland/wayland.h"
    #endif

    /**
     * @def ENGINE_ERROR_CODE_DEFAULT
     * @brief Default error code returned by `engine_error()`, also equal to `EXIT_FAILURE`
     */
    #define ENGINE_ERROR_CODE_DEFAULT EXIT_FAILURE
    /**
     * @def ENGINE_NAME
     * @brief Current name of the engine, used by VkApplicationInfo
     */
    #define ENGINE_NAME "AntaEngine"
    /**
     * @def ENGINE_VERSION
     * Current version of the engine, used by VkApplicationInfo
     */
    #define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)

/**
 * @struct engine
 * @brief Structure representing the engine
 * @var engine::window
 * Active window created by the engine containing its properties and properties of inputs
 * @var engine::camera
 * Active camera rendering the scene
 * @var engine::objects_to_draw
 * Array of objects that will be drawn when `engine_display()` is called.
 * Objects can be added using `engine_draw()`
 * @var engine::objects_to_draw_count
 * Count of objects to draw in the next `engine_display()` call.
 * All objects from indices 0 to `objects_to_draw_count`will be drawn
 * @var engine::max_objects_to_draw
 * Maximum count of objects that can be drawn, it is set upon initialisation in `engine_create()`
 * @var engine::vulkan_context
 * Vulkan context containing all the necessary variables for the vulkan wrapper to work
 */
typedef struct engine
{
    window_t window;
    struct camera camera;

    object_t *objects_to_draw;
    uint32_t objects_to_draw_count;
    uint32_t max_objects_to_draw;

    struct vulkan_context vulkan_context;
} * engine_t;

/**
 * @brief Cleanup the engine and free the allocated memory it allocated.
 * Before the cleanup `engine_wait_idle()` should be called beforehand, making sure all process has ended before cleaning up
 * 
 * @param engine Pointer to the engine structure to end
 */
void engine_cleanup(engine_t engine);
/**
 * @brief Allocate and initialise an engine and its window to their default state and return the allocated engine
 * 
 * @param application_name String pointer of the application's name, it will be used as window's title
 * @param application_version Current version of the application created by the engine
 * @param window_width Original width of the window upon creation
 * @param window_height Original height of the window upon creation
 * @param max_objects_to_draw Maximum allowed objects to draw per call of `engine_display()`, will be used to allocate the sizeof the `objects_to_draw` array from the engine
 * @return engine_t
 */
engine_t engine_create(const char *application_name, const struct version application_version, int window_width, int window_height, uint32_t max_objects_to_draw);
/**
 * @brief Display the objects on the screen and render a frame, also reset the `objects_to_draw_count` to 0
 * 
 * @param engine Pointer to the engine structure that will display
 * @return true if the display worked as intended
 * @return false otherwise
 */
bool engine_display(engine_t engine);
/**
 * @brief Add an object to the array of objects to draw on the next `engine_display()` call,
 * The caller make sure that the object pointer stay valid until `engine_display()` is called.
 * 
 * @param engine Pointer to the engine where the object will be drawn
 * @param object Pointer to the object to draw
 * @return true if the count of objects to draw per `engine_display()` call hasn't reach the maximum set upon creation
 * @return false  if the count of objects to draw per `engine_display()` call has reach the maximum set upon creation
 */
bool engine_draw(engine_t engine, object_t object);
/**
 * @brief Poll window's and input's events
 * It is highly recommended to call it at the top of main loop and once per iteration of the loop
 * 
 * @param engine Pointer to the engine to poll the events
 */
void engine_poll_events(engine_t engine);
/**
 * @brief Returns a boolean if the window or application should close due to internal events, such as closing the window or any critical errors
 * It is recommended to wait for this event to be true before cleaning up the engine with `engine_cleanup`.
 * It is also strongly recommended to end your main loop upon this function returning false
 * 
 * @param engine Pointer to the engine to check if the app should be closed
 * @return true if the application should close
 * @return false if the application's didn't had internal events telling it to close
 */
bool engine_should_close(engine_t engine);
/**
 * @brief Ending an engine activity before cleaning up. Always call this function before any destruction of objects, scenes or cleaning up the engine
 * 
 * @param engine Pointer to the engine that will wait to be idle
 */
void engine_wait_idle(engine_t engine);
/**
 * @brief Update the view and projection of the scene using the main camera of the engine
 * The camera's informations won't be updated in the scene until this function is called
 * 
 * @param engine Pointer to the engine where the camera view and projection will be updated
 */
void engine_update_camera(engine_t engine);

#endif