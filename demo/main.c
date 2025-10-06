#include "raycast.h"
#include "mapgen.h"

#include <stdio.h>
#include <stdlib.h>

#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define PURPLE 0xFFFF00FF
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

int demoMap[] = {
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    BLUE,  RED,   BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    GREEN, GREEN, GREEN, GREEN, GREEN, -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    BLUE,
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

enum Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

void move_camera(RaycastCamera *camera, int direction) {
    float speed = 0.01f;
    if (direction == FORWARD) {
        camera->posX += camera->dirX * speed;
        camera->posY += camera->dirY * speed;
    } else if (direction == BACKWARD) {
        camera->posX -= camera->dirX * speed;
        camera->posY -= camera->dirY * speed;
    } else if (direction == LEFT) {
        camera->posX -= camera->dirY * speed;
        camera->posY += camera->dirX * speed;
    } else if (direction == RIGHT) {
        camera->posX += camera->dirY * speed;
        camera->posY -= camera->dirX * speed;
    }
}

int main(int argc, char *argv[]) {
    int w = 800;
    int h = 600;
    int blockSize = 20;
    int seed = 40;
    int black = 0x00000000;
    int red = 0x00FF0000;
    int purple = 0xFFFF00FF;
    int green = 0xFF00FF00;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo", w, h, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Raycaster *raycaster = raycast_init(w, h);
    RaycastCamera camera = {0, 0, 0.0f, 90.0f, 0.0f, 0.0f, 90};
    float posX; // X coordinate
    float posY; // Y coordinate
    float dirX; // Direction vector x
    float dirY; // Direction vector y
    float planeX; // Camera plane x
    float planeY; // Camera plane y
    int fov; // Field of view in degrees
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
                    case SDL_SCANCODE_W: move_camera(&camera, FORWARD); break;
                    case SDL_SCANCODE_S: move_camera(&camera, BACKWARD); break;
                    case SDL_SCANCODE_A: move_camera(&camera, LEFT); break;
                    case SDL_SCANCODE_D: move_camera(&camera, RIGHT); break;
                    case SDL_SCANCODE_Q:
                        camera.dirX += 10.0f;
/*                        if (camera.dirX < 0.0f) {
                            camera.dirX -= 360.0f;
                        }*/
                        break;
                    case SDL_SCANCODE_E:
                        camera.dirX -= 10.0f;
/*                        if (camera.dirX >= 360.0f) {
                            camera.dirX += 360.0f;
                        }*/
                        break;
                    case SDL_SCANCODE_R:
                        if (dimensions == 2) {
                            dimensions = 3;
                        } else {
                            dimensions = 2;
                        }
                    default: break;
                }
            }
        }

        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (dimensions == 2) {
                raycast_render_2d(raycaster, &camera, renderer, w, &black, &red);
            } else {
                raycast_render(raycaster, &camera, w, h, renderer, &black);
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