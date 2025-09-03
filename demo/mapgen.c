#include "mapgen.h"

#include <stdlib.h>

static const RaycastColor GREEN = 0xFF00FF00;

void generate_map(Raycaster *raycaster, int w, int h, int seed, int blockSize) {
    srand(seed);

    for (int i = 0; i < w * h; i++) {
        raycaster->map[i] = GREEN;
    }

    int point = 0;
    int direction = 1;
    for (int i = 0; i < w * h / blockSize; i++) {
        for (int j = 0; j < blockSize; j++) {
            for (int k = 0; k < blockSize; k++) {
                int idx = point + j + k * w;
                if (idx < 0 || idx >= w * h) {
                    continue;
                }
                raycaster->map[idx] = RAYCAST_EMPTY;
            }
        }
        int r = rand() % 4;
        switch (r) {
            case 0:
                if ((point % w) + blockSize >= w) {
                    direction *= -1;
                }
                point += blockSize * direction;
                break;
            case 1: case 2: point += w; break;
            case 3: point -= w; break;
        }

        if (point < 0) {
            point = 0;
        } else if (point >= w * h) {
            point = w * h - 1;
        }
    }
}
