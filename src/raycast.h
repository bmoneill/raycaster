#ifndef RAYCAST_H
#define RAYCAST_H

#include <SDL3/SDL.h>
#include <stdint.h>

typedef int32_t           RaycastColor; // ARGB format: 0xAARRGGBB
static const RaycastColor RAYCAST_EMPTY = -1;
typedef enum { RAYCAST_FORWARD, RAYCAST_BACKWARD, RAYCAST_LEFT, RAYCAST_RIGHT } RaycastDirection;

/**
 * @struct Raycaster
 * @brief Raycaster structure
 *
 * @param map 1D array representing the 2D map
 * @param width Width of the map
 * @param height Height of the map
 */
typedef struct {
    RaycastColor* map;
    int           width;
    int           height;
} Raycaster;

/**
 * @struct RaycastRect
 * @brief Raycast rectangle structure
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 */
typedef struct {
    float x;
    float y;
    float w;
    float h;
} RaycastRect;

/**
 * @struct RaycastCamera
 * @brief Raycast camera structure
 *
 * @param posX   X coordinate
 * @param posY   Y coordinate
 * @param dirX   Direction vector x
 * @param dirY   Direction vector y
 * @param planeX Camera plane x
 * @param planeY Camera plane y
 * @param fov    Field of view in degrees
 */
typedef struct {
    float posX;
    float posY;
    float dirX;
    float dirY;
    float planeX;
    float planeY;
    int   fov;
} RaycastCamera;

float      raycast_cast(Raycaster*, float, float, float, RaycastColor*);
bool       raycast_collides(Raycaster*, float, float);
void       raycast_destroy(Raycaster*);
void       raycast_draw(Raycaster*, const RaycastRect*, const RaycastColor*);
void       raycast_erase(Raycaster*, const RaycastRect*);
Raycaster* raycast_init(int, int);
int        raycast_init_ptr(Raycaster*, int, int);
void       raycast_move_camera(RaycastCamera*, RaycastDirection);
void       raycast_move_camera_with_collision(Raycaster*, RaycastCamera*, RaycastDirection);
void raycast_render(Raycaster*, const RaycastCamera*, SDL_Renderer*, int, int, const RaycastColor*);
void raycast_render_2d(
    Raycaster*, const RaycastCamera*, SDL_Renderer*, int, const RaycastColor*, const RaycastColor*);
void raycast_rotate_camera(RaycastCamera*, float);
void raycast_set_draw_color(SDL_Renderer*, const RaycastColor*);
#endif
