#ifndef RAYCAST_H
#define RAYCAST_H

#include <SDL3/SDL.h>
#include <stdint.h>

#ifndef LIBRAYCAST_VERSION
#define LIBRAYCAST_VERSION "unknown"
#endif

typedef int32_t           RaycastColor; // ARGB format: 0xAARRGGBB
static const RaycastColor RAYCAST_EMPTY = -1;
typedef enum { RAYCAST_FORWARD, RAYCAST_BACKWARD, RAYCAST_LEFT, RAYCAST_RIGHT } RaycastDirection;

/**
 * @struct RaycastTexture
 * @brief Texture structure for wall rendering
 *
 * @param pixels ARGB pixel data
 * @param width Width of the texture
 * @param height Height of the texture
 */
typedef struct {
    RaycastColor* pixels;
    int           width;
    int           height;
} RaycastTexture;

/**
 * @struct RaycastHit
 * @brief Information about a raycast hit
 *
 * @param distance Distance to the hit point
 * @param wallX Position where the wall was hit (0.0 to 1.0)
 * @param side Which side of the wall was hit (0 = vertical, 1 = horizontal)
 * @param textureId ID of the texture to use
 */
typedef struct {
    float distance;
    float wallX;
    int   side;
    int   textureId;
} RaycastHit;

/**
 * @struct Raycaster
 * @brief Raycaster structure
 *
 * @param map 1D array representing the 2D map (RaycastColor if untextured, RaycastTexture if textured)
 * @param width Width of the map
 * @param height Height of the map
 * @param textures Array of textures
 * @param textureCount Number of textures
 * @param textured Whether to use textures
 */
typedef struct {
    RaycastColor*    map;
    int              width;
    int              height;
    RaycastTexture** textures;
    int              textureCount;
    int              textured;
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

float           raycast_cast(Raycaster*, float, float, float, RaycastColor*);
void            raycast_cast_textured(Raycaster*, float, float, float, RaycastHit*);
RaycastTexture* raycast_texture_create(int, int);
void            raycast_texture_destroy(RaycastTexture*);
void            raycast_add_texture(Raycaster*, RaycastTexture*);
bool            raycast_collides(Raycaster*, float, float);
void            raycast_destroy(Raycaster*);
void            raycast_draw(Raycaster*, const RaycastRect*, const RaycastColor*);
void            raycast_erase(Raycaster*, const RaycastRect*);
Raycaster*      raycast_init(int, int);
int             raycast_init_ptr(Raycaster*, int, int);
void            raycast_move_camera(RaycastCamera*, RaycastDirection, float);
void raycast_move_camera_with_collision(Raycaster*, RaycastCamera*, RaycastDirection, float);
void raycast_render(Raycaster*, const RaycastCamera*, SDL_Renderer*, int, int, const RaycastColor*);
void raycast_render_textured(
    Raycaster*, const RaycastCamera*, SDL_Renderer*, int, int, const RaycastColor*);
void        raycast_render_2d(Raycaster*,
                              const RaycastCamera*,
                              SDL_Renderer*,
                              int,
                              float,
                              const RaycastColor*,
                              const RaycastColor*,
                              const RaycastColor*);
void        raycast_rotate_camera(RaycastCamera*, float);
void        raycast_set_draw_color(SDL_Renderer*, const RaycastColor*);
const char* raycast_version(void);

#endif
