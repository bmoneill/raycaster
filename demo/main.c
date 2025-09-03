#include "raycast.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo",
                                          800, 600,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    int running = 1;

    Raycaster *raycaster = raycast_init(800, 600);
    RaycasterCamera camera = {{400, 300}, 0.0f, 90.0f};
    RaycasterDimensions dims = {800, 600};
    raycast_draw(raycaster, &(RaycasterRect){{50, 30}, {100, 60}}, (RaycasterColor[]){0xFFFF0000});
    raycast_draw(raycaster, &(RaycasterRect){{250, 150}, {60, 60}}, (RaycasterColor[]){0xFFFF0000});

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        raycast_render_2d(raycaster, &camera, &dims, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    raycast_destroy(raycaster);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}