#include "raycast/raycast.h"
#include "util.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


static void demo             (int,  int,            int,        int);
static int* expand_map       (int*, int,            int,        int,  int);

int demoMap[] = {
    GREEN,  GREEN,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    BLUE,   -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     BLUE,
    BLUE,   PURPLE, BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   -1,     BLUE,
    PURPLE, -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     BLUE,
    PURPLE, -1,     GREEN,  GREEN,  GREEN,  GREEN,  GREEN,  -1,     -1,     BLUE,
    PURPLE, -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     BLUE,
    BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,   BLUE,
};

int main(int argc, char *argv[]) {
    int w = 800;
    int h = 600;
    RaycastColor fg = RED;
    RaycastColor bg = BLACK;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:f:b:v")) != -1) {
        switch (opt) {
            case 'w': w = atoi(optarg); break;
            case 'h': h = atoi(optarg); break;
            case 'f': fg = (RaycastColor) strtol(optarg, NULL, 16); break;
            case 'b': bg = (RaycastColor) strtol(optarg, NULL, 16); break;
            case 'v': printf("libraycast %s\n", raycast_version()); return 0;
            default:
                fprintf(stderr, "Usage: %s [-w width] [-h height] [-f foreground_color] [-b background_color]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    demo(w, h, fg, bg);
    return 0;
}

static void demo(int w, int h, int fg, int bg) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo", w, h, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    int keys[SDL_SCANCODE_COUNT] = {0};

    Raycaster *raycaster = raycast_init(w, h);
    RaycastCamera camera = {w/10 + 10, h/6+10, 0.0f, 90.0f, 0.0f, 0.0f, 90};
    raycaster->map = expand_map(demoMap, 10, 6, w, h);

    int running = 1;
    int draw = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                keys[event.key.scancode] = 1;
            } else if (event.type == SDL_EVENT_KEY_UP) {
                keys[event.key.scancode] = 0;
            }
        }
        handle_keypresses(keys, &camera, raycaster, &draw);

        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            raycast_render(raycaster, &camera, renderer, w, h, &bg);
            raycast_render_2d(raycaster, &camera, renderer, w, 0.2, &bg, NULL, &fg);
            SDL_RenderPresent(renderer);
            draw = 0;
        }

        SDL_Delay(16);
    }

    raycast_destroy(raycaster);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static int* expand_map(int *map, int w, int h, int nw, int nh) {
    int *newMap = (int *) malloc((nw * nh) * sizeof(int));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int color = map[y * w + x];
            for (int j = 0; j < nh / h; j++) {
                for (int i = 0; i < nw / w; i++) {
                    newMap[(y * (nh / h) + j) * nw + (x * (nw / w) + i)] = color;
                }
            }
        }
    }
    return newMap;
}
