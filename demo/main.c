#include "raycast.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster Demo",
                                          800, 600,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    int running = 1;
    int draw = 1;

    Raycaster *raycaster = raycast_init(800, 600);
    RaycasterCamera camera = {{450, 300}, 180.0f, 90.0f};
    RaycasterDimensions dims = {800, 600};
    raycast_draw(raycaster, &(RaycasterRect){{50, 100}, {100, 60}}, (RaycasterColor[]){0x00FF00FF});
    raycast_draw(raycaster, &(RaycasterRect){{250, 150}, {60, 60}}, (RaycasterColor[]){0x00FF00FF});

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        int w, h;

        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            raycast_render_2d(raycaster, &camera, &dims, renderer);
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