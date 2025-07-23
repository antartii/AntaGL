#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/cglm.h>
#define CAMERA_DEPTH_MIN_RENDER 0.0f
#define CAMERA_DEPTH_MAX_RENDER 10.0f
#define CAMERA_FOV_DEFAULT_DEGREES 45.0f

typedef struct camera {
    // view
    vec3 pos;
    vec3 target;
    vec3 up;

    // projection
    vec2 render_depth_range;
    float fov; // in radians
    
} * camera_t;

void camera_init(camera_t camera);

#endif
