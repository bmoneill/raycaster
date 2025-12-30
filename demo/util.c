#include "util.h"

void handle_keypresses(int* keys, RaycastCamera* camera, Raycaster* raycaster, int* draw) {
    float speed = 0.5f;
    if (keys[SDL_SCANCODE_W]) {
        raycast_move_camera_with_collision(raycaster, camera, RAYCAST_FORWARD, speed);
        *draw = 1;
    }
    if (keys[SDL_SCANCODE_S]) {
        raycast_move_camera_with_collision(raycaster, camera, RAYCAST_BACKWARD, speed);
        *draw = 1;
    }
    if (keys[SDL_SCANCODE_A]) {
        raycast_move_camera_with_collision(raycaster, camera, RAYCAST_LEFT, speed);
        *draw = 1;
    }
    if (keys[SDL_SCANCODE_D]) {
        raycast_move_camera_with_collision(raycaster, camera, RAYCAST_RIGHT, speed);
        *draw = 1;
    }
    if (keys[SDL_SCANCODE_Q]) {
        raycast_rotate_camera(camera, -0.1f);
        *draw = 1;
    }
    if (keys[SDL_SCANCODE_E]) {
        raycast_rotate_camera(camera, 0.1f);
        *draw = 1;
    }
}
