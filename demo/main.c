#include "raycast.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define PURPLE 0xFFFF00FF
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

int demoMap[] = {
    GREEN, GREEN, PURPLE, PURPLE, PURPLE,  PURPLE,  PURPLE,  PURPLE,  PURPLE,  PURPLE,
    BLUE,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    BLUE,
    BLUE,  RED,   BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  -1,    BLUE,
    RED,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    BLUE,
    RED,    -1,    GREEN, GREEN, GREEN, GREEN, GREEN, -1,    -1,    BLUE,
    PURPLE,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    BLUE,
    BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,
};

int* expand_map(int *map, int w, int h, int nw, int nh) {
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

int main(int argc, char *argv[]) {
    int w = 800;
    int h = 600;
    RaycastColor fg = RED;
    RaycastColor bg = BLACK;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo", w, h, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Raycaster *raycaster = raycast_init(w, h);
    RaycastCamera camera = {800/10 + 10, 800/6, 0.0f, 90.0f, 0.0f, 0.0f, 90};
    raycaster->map = expand_map(demoMap, 10, 6, w, h);

    int running = 1;
    int draw = 1;
    int dimensions = 2; // 2D or 3D rendering mode
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                draw = 1;
                switch (event.key.scancode) {
                    case SDL_SCANCODE_W: raycast_move_camera_with_collision(raycaster, &camera, RAYCAST_FORWARD); break;
                    case SDL_SCANCODE_S: raycast_move_camera_with_collision(raycaster, &camera, RAYCAST_BACKWARD); break;
                    case SDL_SCANCODE_A: raycast_move_camera_with_collision(raycaster, &camera, RAYCAST_LEFT); break;
                    case SDL_SCANCODE_D: raycast_move_camera_with_collision(raycaster, &camera, RAYCAST_RIGHT); break;
                    case SDL_SCANCODE_Q: raycast_rotate_camera(&camera, -0.1f); break;
                    case SDL_SCANCODE_E: raycast_rotate_camera(&camera, 0.1f); break;
                    case SDL_SCANCODE_R: dimensions = dimensions == 2 ? 3 : 2; break;
                    default: break;
                }
            }
        }

        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (dimensions == 2) {
                raycast_render_2d(raycaster, &camera, renderer, w, &bg, &fg);
            } else {
                raycast_render(raycaster, &camera, renderer, w, h, &bg);
            }
            SDL_RenderPresent(renderer);
            draw = 0;
        }

        SDL_Delay(16);
    }

    raycast_destroy(raycaster);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}