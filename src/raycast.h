#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>
#include <SDL3/SDL.h>

#define RAYCASTER_BLACK 0x00000000

typedef int32_t RaycasterColor;

typedef struct {
    float x;
    float y;
} RaycasterPoint;

typedef struct {
    int w;
    int h;
} RaycasterDimensions;

typedef struct {
    RaycasterColor *map;
    RaycasterDimensions size;
} Raycaster;

typedef struct {
    RaycasterPoint point;
    RaycasterDimensions size;
} RaycasterRect;

typedef struct {
    RaycasterPoint position;
    float direction;
    float fov;
} RaycasterCamera;

Raycaster *raycast_init(int w, int h);
int raycast_init_ptr(Raycaster *raycaster, int w, int h);
void raycast_destroy(Raycaster *raycaster);
void raycast_draw(Raycaster *raycaster, RaycasterRect *rect, RaycasterColor *color);
void raycast_erase(Raycaster *raycaster, RaycasterRect *rect);
void raycast_render(Raycaster *raycaster, int displayWidth, int displayHeight);
void raycast_render_2d(Raycaster *raycaster, RaycasterCamera *camera, RaycasterDimensions *dimensions, SDL_Renderer *renderer);

#endif
