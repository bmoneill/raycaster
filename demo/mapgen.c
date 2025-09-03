#include "mapgen.h"

#include <stdlib.h>

void generate_map(Raycaster *raycaster, int w, int h, int seed, int blockSize) {
    const RaycastColor GREEN = 0xFF00FF00;

    srand(seed);
    memset(raycaster->map, GREEN, w * h * sizeof(RaycastColor));

    int point = 0;
    for (int i = 0; i < w * h / blockSize; i++) {
        raycaster->map[point] = RAYCAST_EMPTY;
        int r = rand() % 3;
        switch (r) {
            case 0: point += blockSize; break;
            case 1: point += w; break;
            case 2: point -= blockSize; break;
        }

        if (point < 0) {
            point = 0;
        } else if (point >= w * h) {
            point = w * h - 1;
        }
    }
}
