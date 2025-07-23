#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/cglm.h>

typedef struct camera {
    // view
    vec3 pos;
    vec3 target;
    vec3 up;

    // projection
    
} * camera_t;

void camera_init(camera_t camera);

#endif
