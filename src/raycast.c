#include "raycast.h"

#include <stdlib.h>

Raycaster *raycast_init(int w, int h) {
    Raycaster *raycaster = (Raycaster *) malloc(sizeof(Raycaster));
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

    raycaster->w = w;
    raycaster->h = h;
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
    // TODO Implement
}

void raycast_erase(Raycaster *raycaster, RaycasterRect *rect) {
    // TODO Implement
}

void raycast_render(Raycaster *raycaster) {
    // TODO Implement
}
