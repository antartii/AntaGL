#include "camera.h"

void camera_init(camera_t camera)
{
    glm_vec3_copy((vec3) {2.0f, 2.0f, 2.0f}, camera->pos);
    glm_vec3_copy((vec3) {0.0f, 0.0f, 0.0f}, camera->target);
    glm_vec3_copy((vec3) {0.0f, 0.0f, 1.0f}, camera->up);
}
