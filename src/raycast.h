#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

typedef struct {
    int *map;
} Raycaster;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} RaycasterRect;

typedef int32_t RaycasterColor;

int raycast_init(Raycaster *raycaster, int w, int h);
void raycast_destroy(Raycaster *raycaster);
void raycast_draw(Raycaster *raycaster, RaycasterRect *rect, RaycasterColor *color);
void raycast_erase(Raycaster *raycaster, RaycasterRect *rect);
void raycast_render(Raycaster *raycaster);

#endif
