#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

typedef int32_t RaycasterColor;

typedef struct {
    RaycasterColor *map;
    int w;
    int h;
} Raycaster;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} RaycasterRect;


Raycaster *raycast_init(int w, int h);
int raycast_init_ptr(Raycaster *raycaster, int w, int h);
void raycast_destroy(Raycaster *raycaster);
void raycast_draw(Raycaster *raycaster, RaycasterRect *rect, RaycasterColor *color);
void raycast_erase(Raycaster *raycaster, RaycasterRect *rect);
void raycast_render(Raycaster *raycaster);

#endif
