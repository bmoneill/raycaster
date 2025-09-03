#include "raycast.h"

#include <stdlib.h>

Raycaster *raycast_init(int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);

    Raycaster *raycaster = (Raycaster *) calloc(1, sizeof(Raycaster));
    if (!raycaster) {
        return NULL;
    }

    int result = raycast_init_ptr(raycaster, w, h);

    if (result) {
        free(raycaster);
        return NULL;
    }

    return raycaster;
}

int raycast_init_ptr(Raycaster *raycaster, int w, int h) {
    if (raycaster->map) {
        free(raycaster->map);
    }

    raycaster->map = (RaycasterColor *) calloc(w * h, sizeof(RaycasterColor));
    if (!raycaster->map) {
        return 1;
    }

    raycaster->size.w = w;
    raycaster->size.h = h;
    return 0;
}

void raycast_destroy(Raycaster *raycaster) {
    if (raycaster) {
        if (raycaster->map) {
            free(raycaster->map);
        }
        free(raycaster);
    }
}

void raycast_draw(Raycaster *raycaster, RaycasterRect *rect, RaycasterColor *color) {
    for (int i = 0; i < rect->size.h; i++) {
        for (int j = 0; j < rect->size.w; j++) {
            if (rect->point.x + j < 0 || rect->point.x + j >= raycaster->size.w ||
                rect->point.y + i < 0 || rect->point.y + i >= raycaster->size.h) {
                continue;
            }
            raycaster->map[((int) rect->point.y + i) * raycaster->size.w + ((int) rect->point.x + j)] = *color;
        }
    }
}

void raycast_erase(Raycaster *raycaster, RaycasterRect *rect) {
    raycast_draw(raycaster, rect, (RaycasterColor[]){RAYCASTER_BLACK});
}

void raycast_render(Raycaster *raycaster, int displayWidth, int displayHeight) {
    // TODO Implement
}

void raycast_render_2d(Raycaster *raycaster, RaycasterCamera *camera, RaycasterDimensions *dimensions, SDL_Renderer *renderer) {
    for (int y = 0; y < raycaster->size.h; y++) {
        for (int x = 0; x < raycaster->size.w; x++) {
            RaycasterColor color = raycaster->map[y * raycaster->size.w + x];
            SDL_SetRenderDrawColor(renderer,
                                   (color >> 16) & 0xFF,
                                   (color >> 8) & 0xFF,
                                   color & 0xFF,
                                   (color >> 24) & 0xFF);
            SDL_RenderPoint(renderer, x, y);
        }
    }

    // TODO implement camera
}
