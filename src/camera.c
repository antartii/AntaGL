#include "camera.h"

void camera_init(camera_t camera)
{
    glm_vec3_copy((vec3) {0.0f, 0.0f, 2.0f}, camera->pos);
    glm_vec3_copy((vec3) {0.0f, 0.0f, 0.0f}, camera->target);
    glm_vec3_copy((vec3) {0.0f, 1.0f, 0.0f}, camera->up);

    glm_vec2_copy((vec2) {CAMERA_DEPTH_MIN_RENDER_DEFAULT, CAMERA_DEPTH_MAX_RENDER_DEFAULT}, camera->render_depth_range);
    camera->fov_in_radians = CAMERA_FOV_DEFAULT_DEGREES;
}
