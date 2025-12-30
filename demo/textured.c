#include "raycast/raycast.h"
#include "util.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAMP(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (x)))

static RaycastTexture* create_brick_texture(int, int);
static RaycastTexture* create_checkered_texture(int, int);
static RaycastTexture* create_stone_texture(int, int);
static RaycastTexture* create_wood_texture(int, int);

int                    texturedDemoMap[] = {
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, 0,  0,  0,  0,  -1, -1, -1, -1, -1,
    -1, 2,  2,  2,  2,  -1, -1, 1,  1,  -1, -1, 0,  -1, -1, 0,  -1, -1, -1, -1, -1, -1, 2,  -1, -1,
    2,  -1, -1, 1,  1,  -1, -1, 0,  -1, -1, 0,  -1, -1, -1, -1, -1, -1, 2,  -1, -1, 2,  -1, -1, 1,
    1,  -1, -1, 0,  0,  0,  0,  -1, -1, -1, -1, -1, -1, 2,  2,  2,  2,  -1, -1, 1,  1,  -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1,
    -1, 3,  3,  -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1, -1, 3,  3,  -1,
    -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1, -1, 3,  3,  -1, -1, -1, -1, -1,
    -1, -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1, -1, 3,  3,  -1, -1, -1, -1, -1, -1, -1, -1, 1,
    1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  -1, -1, 2,
    2,  2,  -1, -1, -1, -1, -1, -1, -1, -1, 0,  0,  0,  -1, -1, 1,  1,  -1, -1, 2,  -1, 2,  -1, -1,
    -1, -1, -1, -1, -1, -1, 0,  -1, 0,  -1, -1, 1,  1,  -1, -1, 2,  -1, 2,  -1, -1, -1, -1, -1, -1,
    -1, -1, 0,  -1, 0,  -1, -1, 1,  1,  -1, -1, 2,  2,  2,  -1, -1, -1, -1, -1, -1, -1, -1, 0,  0,
    0,  -1, -1, 1,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,
    1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
};

int main(int argc, char* argv[]) {
    int             w                        = 800;
    int             h                        = 600;
    int             mapWidth                 = 20;
    int             mapLength                = 20;
    RaycastColor    fg                       = RED;
    RaycastColor    bg                       = BLACK;
    RaycastColor    bg2D                     = WHITE;
    RaycastColor    wall2D                   = BLUE;
    SDL_Window*     window                   = NULL;
    SDL_Renderer*   renderer                 = NULL;
    Raycaster*      raycaster                = NULL;
    RaycastTexture* brickTexture             = create_brick_texture(64, 64);
    RaycastTexture* stoneTexture             = create_stone_texture(64, 64);
    RaycastTexture* woodTexture              = create_wood_texture(64, 64);
    RaycastTexture* checkerTexture           = create_checkered_texture(64, 64);
    int             running                  = 1;
    int             keys[SDL_SCANCODE_COUNT] = { 0 };
    int             draw                     = 1;
    RaycastCamera   camera                   = { .posX   = 11.0f,
                                                 .posY   = 11.5f,
                                                 .dirX   = 1.0f,
                                                 .dirY   = 0.0f,
                                                 .planeX = 0.0f,
                                                 .planeY = 0.66f,
                                                 .fov    = 90 };
    SDL_Event       event;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (!(window = SDL_CreateWindow("Raycaster Textured Demo", w, h, SDL_WINDOW_RESIZABLE))) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    if (!(renderer = SDL_CreateRenderer(window, NULL))) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!(raycaster = raycast_init(mapWidth, mapLength))) {
        fprintf(stderr, "Failed to create raycaster\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    raycaster->textured = 1;
    raycast_add_texture(raycaster, brickTexture); // 0
    raycast_add_texture(raycaster, stoneTexture); // 1
    raycast_add_texture(raycaster, woodTexture); // 2
    raycast_add_texture(raycaster, checkerTexture); // 3

    for (int i = 0; i < mapWidth * mapLength; i++) {
        raycaster->map[i] = texturedDemoMap[i];
    }

    while (running) {
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
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            raycast_render_textured(raycaster, &camera, renderer, w, h, &bg);
            raycast_render_2d(raycaster, &camera, renderer, mapWidth, 2.0, &bg2D, &wall2D, &fg);
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

/**
 * @brief Create a brick texture pattern
 *
 * @param width Width of the texture
 * @param height Height of the texture
 * @return RaycastTexture* The created brick texture
 */
static RaycastTexture* create_brick_texture(int width, int height) {
    RaycastTexture* texture = raycast_texture_create(width, height);
    if (!texture)
        return NULL;

    RaycastColor brick_color  = 0xFF8B4513;
    RaycastColor mortar_color = 0xFFD3D3D3;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (y % 8 == 0 || y % 8 == 7) {
                texture->pixels[y * width + x] = mortar_color;
            } else if ((y / 8) % 2 == 0) {
                if (x % 16 == 0 || x % 16 == 15) {
                    texture->pixels[y * width + x] = mortar_color;
                } else {
                    texture->pixels[y * width + x] = brick_color;
                }
            } else {
                if ((x + 8) % 16 == 0 || (x + 8) % 16 == 15) {
                    texture->pixels[y * width + x] = mortar_color;
                } else {
                    texture->pixels[y * width + x] = brick_color;
                }
            }
        }
    }

    return texture;
}

/**
 * @brief Create a checkered texture pattern
 *
 * @param width Width of the texture
 * @param height Height of the texture
 * @return RaycastTexture* The created checkered texture
 */
static RaycastTexture* create_checkered_texture(int width, int height) {
    RaycastTexture* texture = raycast_texture_create(width, height);
    if (!texture)
        return NULL;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if ((x / 8 + y / 8) % 2 == 0) {
                texture->pixels[y * width + x] = WHITE;
            } else {
                texture->pixels[y * width + x] = BLACK;
            }
        }
    }

    return texture;
}

/**
 * @brief Create a stone texture pattern
 *
 * @param width Width of the texture
 * @param height Height of the texture
 * @return RaycastTexture* The created stone texture
 */
static RaycastTexture* create_stone_texture(int width, int height) {
    RaycastTexture* texture = raycast_texture_create(width, height);
    if (!texture)
        return NULL;

    RaycastColor base_color = 0xFF808080;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int variation                  = ((x * 7 + y * 13) % 32) - 16;
            int r                          = CLAMP(((base_color >> 16) & 0xFF) + variation);
            int g                          = CLAMP(((base_color >> 8) & 0xFF) + variation);
            int b                          = CLAMP((base_color & 0xFF) + variation);
            texture->pixels[y * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    return texture;
}

/**
 * @brief Create a wood texture pattern
 *
 * @param width Width of the texture
 * @param height Height of the texture
 * @return RaycastTexture* The created wood texture
 */
static RaycastTexture* create_wood_texture(int width, int height) {
    RaycastTexture* texture = raycast_texture_create(width, height);
    if (!texture)
        return NULL;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create wood grain effect using sine wave
            float grain                    = sinf(x * 0.5f) * 20.0f;
            int   r                        = CLAMP(139 + (int) grain); // Base brown color
            int   g                        = CLAMP(90 + (int) grain);
            int   b                        = CLAMP(43 + (int) (grain * 0.5f));
            texture->pixels[y * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    return texture;
}
