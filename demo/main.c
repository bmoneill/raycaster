#include "raycast.h"
#include "mapgen.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int w = 800;
    int h = 600;
    int blockSize = 10;
    int seed = 42;
    int black = 0x00000000;
    int red = 0x00FF0000;
    int purple = 0xFFFF00FF;
    int green = 0xFF00FF00;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo", w, h, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Raycaster *raycaster = raycast_init(w, h);
    RaycastDimensions dims = {w, h};
    RaycastCamera camera = {{0, 0}, 0.0f, 90.0f};
    generate_map(raycaster, w, h, seed, blockSize);

    int running = 1;
    int draw = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                draw = 1;
                switch (event.key.scancode) {
                    case SDL_SCANCODE_W: camera.position.y -= 1; break;
                    case SDL_SCANCODE_S: camera.position.y += 1; break;
                    case SDL_SCANCODE_A: camera.position.x -= 1; break;
                    case SDL_SCANCODE_D: camera.position.x += 1; break;
                    case SDL_SCANCODE_Q:
                        camera.direction -= 1.0f;
                        if (camera.direction < 0.0f) {
                            camera.direction += 360.0f;
                        }
                        break;
                    case SDL_SCANCODE_E:
                        camera.direction += 1.0f;
                        if (camera.direction >= 360.0f) {
                            camera.direction -= 360.0f;
                        }
                        break;
                    default: break;
                }
            }
        }

        int w, h;
        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            raycast_render_2d(raycaster, &camera, &dims, renderer, &black, &red);
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