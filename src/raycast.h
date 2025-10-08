#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>
#include <SDL3/SDL.h>

typedef int32_t RaycastColor; // ARGB format: 0xAARRGGBB

static const RaycastColor RAYCAST_EMPTY = -1;

typedef enum {
    RAYCAST_FORWARD,
    RAYCAST_BACKWARD,
    RAYCAST_LEFT,
    RAYCAST_RIGHT
} RaycastDirection;

typedef struct {
    RaycastColor *map; // 1D array representing the 2D map
    int width;
    int height;
} Raycaster;

typedef struct {
    float x; // X coordinate
    float y; // Y coordinate
    float w; // Width
    float h; // Height
} RaycastRect;

typedef struct {
    float posX; // X coordinate
    float posY; // Y coordinate
    float dirX; // Direction vector x
    float dirY; // Direction vector y
    float planeX; // Camera plane x
    float planeY; // Camera plane y
    int fov; // Field of view in degrees
} RaycastCamera;

float      raycast_cast                      (Raycaster*,          float,
                                              float,               float,
                                              RaycastColor*);
bool       raycast_collides                  (Raycaster*,          float,
                                              float);
void       raycast_destroy                   (Raycaster*);
void       raycast_draw                      (Raycaster*,          const RaycastRect*,
                                              const RaycastColor*);
void       raycast_erase                     (Raycaster*,          const RaycastRect*);
Raycaster* raycast_init                      (int,                 int);
int        raycast_init_ptr                  (Raycaster*,          int,
                                              int);
void       raycast_move_camera               (RaycastCamera*,      RaycastDirection);
void       raycast_move_camera_with_collision(Raycaster*,          RaycastCamera*,
                                              RaycastDirection);
void       raycast_render                    (Raycaster*,          const RaycastCamera*,
                                              SDL_Renderer*,       int,
                                              int,                 const RaycastColor*);
void       raycast_render_2d                 (Raycaster*,          const RaycastCamera*,
                                              SDL_Renderer*,       int,
                                              float,               const RaycastColor*,
                                              const RaycastColor*);
void       raycast_rotate_camera             (RaycastCamera*,      float);
void       raycast_set_draw_color            (SDL_Renderer*,       const RaycastColor *);
#endif
