#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>
#include <SDL3/SDL.h>

typedef int32_t RaycastColor; // ARGB format: 0xAARRGGBB

const RaycastColor RAYCAST_EMPTY = -1;

typedef struct {
    float x;
    float y;
} RaycastPoint;

typedef struct {
    int w;
    int h;
} RaycastDimensions;

typedef struct {
    RaycastColor *map; // 1D array representing the 2D map
    RaycastDimensions size; // Size of the entire map
} Raycaster;

typedef struct {
    RaycastPoint point; // Point from top-left (x, y)
    RaycastDimensions size; // Size (width, height)
} RaycastRect;

typedef struct {
    RaycastPoint position; // Position (x, y)
    float direction; // Direction (deg)
    float fov; // Field of view (deg)
} RaycastCamera;

Raycaster *raycast_init(int, int);
int raycast_init_ptr(Raycaster *, int, int);
float raycast_cast(Raycaster *, const RaycastPoint *, float);
bool raycast_collides(Raycaster *, const RaycastPoint *);
void raycast_destroy(Raycaster *);
void raycast_draw(Raycaster *, const RaycastRect *, const RaycastColor *);
void raycast_erase(Raycaster *, const RaycastRect *);
void raycast_render(Raycaster *, int, int);
void raycast_render_2d(Raycaster *, const RaycastCamera *, const RaycastDimensions *, SDL_Renderer *, const RaycastColor *, const RaycastColor *);
void raycast_set_draw_color(SDL_Renderer *, const RaycastColor *);

#endif
