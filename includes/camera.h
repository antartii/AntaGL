#ifndef _CAMERA_H
    #define _CAMERA_H

    #include <cglm/cglm.h>

    /**
     * @def CAMERA_DEPTH_MIN_RENDER_DEFAULT
     * @brief Default value for the minimum depth rendered by the camera
     */
    #define CAMERA_DEPTH_MIN_RENDER_DEFAULT 0.0f
    /**
     * @def CAMERA_DEPTH_MAX_RENDER_DEFAULT
     * @brief Default value for the maximum depth rendered by the camera
     */
    #define CAMERA_DEPTH_MAX_RENDER_DEFAULT 10.0f
    /**
     * @def CAMERA_FOV_DEFAULT_DEGREES
     * @brief Default value for the camera field of view in degrees
     */
    #define CAMERA_FOV_DEFAULT_DEGREES glm_rad(45.0f)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct camera
 * @brief Structure representing a camera object in a 3D space
 * @var camera::pos
 * Position of the camera in the 3D space
 * @var camera::target
 * Position a virtual point in the 3D space where the camera is looking
 * @var camera::up
 * Direction of where the top edge of the camera should be
 * @var camera::render_depth_range
 * Range of rendering, objects or part of objects outside of this range won't be rendered
 * @var camera::fov_in_radians
 * Field of view in radians of the camera
 */
typedef struct camera {
    // view
    vec3 pos;
    vec3 target;
    vec3 up;

    // projection
    vec2 render_depth_range;
    float fov_in_radians;
} * camera_t;

/**
 * @brief Initialize a camera to default values
 * 
 * @param camera Pointer to a camera that will be initialize to its default values
 */
void camera_init(camera_t camera);

#ifdef __cplusplus
    }
#endif

#endif
