#include "raycast.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Initialize a Raycaster instance.
 *
 * This function allocates a new Raycaster instance and initializes it with the specified width and height.
 *
 * @param w The width of the Raycaster map.
 * @param h The height of the Raycaster map.
 * @return The newly allocated Raycaster instance, or NULL on failure.
 */
Raycaster *raycast_init(int w, int h) {
    Raycaster *raycaster = (Raycaster *) calloc(1, sizeof(Raycaster));
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

/**
 * @brief (Re-)Initialize an allocated raycaster instance.
 *
 * This function initializes or re-initializes a pre-allocated Raycaster instance with the specified width and height.
 * If the instance already has an allocated map, it will be freed before allocating a new one.
 *
 * @param raycaster The Raycaster to initialize.
 * @param w The width of the Raycaster map.
 * @param h The height of the Raycaster map.
 * @return 0 on success, 1 on memory allocation failure.
 */
int raycast_init_ptr(Raycaster *raycaster, int w, int h) {
    if (raycaster->map) {
        free(raycaster->map);
    }

    raycaster->map = (RaycastColor *) malloc(w * h * sizeof(RaycastColor));
    if (!raycaster->map) {
        return 1;
    }
    memset(raycaster->map, -1, w * h * sizeof(RaycastColor));

    raycaster->size.w = w;
    raycaster->size.h = h;
    return 0;
}

/**
 * @brief Destroy a Raycaster instance.
 *
 * This function frees the memory allocated for the Raycaster instance and its map.
 *
 * @param raycaster The Raycaster instance to destroy.
 */
void raycast_destroy(Raycaster *raycaster) {
    if (raycaster) {
        if (raycaster->map) {
            free(raycaster->map);
        }
        free(raycaster);
    }
}

/**
 * @brief Draw a rectangle on the Raycaster map.
 *
 * This function fills a rectangle area on the Raycaster map with the specified color.
 * If the rectangle exceeds the bounds of the map, it will be clipped accordingly.
 *
 * @param raycaster The Raycaster instance to draw on.
 * @param rect The rectangle to draw, defined by its top-left point and size.
 * @param color The color to fill the rectangle with.
 */
void raycast_draw(Raycaster *raycaster, const RaycastRect *rect, const RaycastColor *color) {
    for (int i = 0; i < rect->size.h; i++) {
        for (int j = 0; j < rect->size.w; j++) {
            if (rect->point.x + j < 0 || rect->point.x + j >= raycaster->size.w ||
                rect->point.y + i < 0 || rect->point.y + i >= raycaster->size.h) {
                continue;
            }
            raycaster->map[((int) rect->point.y + i) * raycaster->size.w + ((int) rect->point.x + j)] = *color;
        }
    }
}

/**
 * @brief Erase a rectangle area on the Raycaster map.
 *
 * This function fills a rectangle area on the Raycaster map with the background color (black).
 *
 * @param raycaster The Raycaster instance to erase from.
 * @param rect The rectangle area to erase, defined by its top-left point and size.
 */
void raycast_erase(Raycaster *raycaster, const RaycastRect *rect) {
    raycast_draw(raycaster, rect, &RAYCAST_EMPTY);
}

/**
 * @brief Render the Raycaster map to the display.
 *
 * This function is not yet implemented.
 *
 * @param raycaster The Raycaster instance to render.
 * @param displayWidth The width of the display area.
 * @param displayHeight The height of the display area.
 * @param renderer The SDL_Renderer to use for rendering.
 * @param background The background color to use for empty spaces.
 */
void raycast_render(Raycaster *raycaster, const RaycastCamera *camera, const RaycastDimensions *dimensions,
                       SDL_Renderer *renderer, const RaycastColor *background) {
    // TODO Implement
}

/**
 * @brief Render the Raycaster map in 2D mode to the display.
 *
 * This function renders the Raycaster map in a 2D view using the provided SDL_Renderer.
 * The camera logic is currently not implemented.
 *
 * @param raycaster The Raycaster instance to render.
 * @param camera The camera settings for rendering (currently unused).
 * @param dimensions The dimensions of the rendering area (currently unused).
 * @param renderer The SDL_Renderer to use for rendering.
 * @param background The background color to use for empty spaces.
 * @param rayColor The color to use for rendering rays.
 */
void raycast_render_2d(Raycaster *raycaster, const RaycastCamera *camera, const RaycastDimensions *dimensions,
                       SDL_Renderer *renderer, const RaycastColor *background, const RaycastColor *rayColor,
                       int x, int y) {
    // Render the map
    for (int y = 0; y < raycaster->size.h; y++) {
        for (int x = 0; x < raycaster->size.w; x++) {
            RaycastColor color = raycaster->map[y * raycaster->size.w + x];
            raycast_set_draw_color(renderer, (color == RAYCAST_EMPTY) ? background : &color);
            SDL_RenderPoint(renderer, x, y);
        }
    }

    // Render the rays
    RaycastColor hit = RAYCAST_EMPTY;
    raycast_set_draw_color(renderer, rayColor);
    float startX = camera->direction - (camera->fov / 2);
    float endX = camera->direction + (camera->fov / 2);
    for (float angle = startX; angle <= endX; angle += (camera->fov * 2) / dimensions->w) {
        float distance = raycast_cast(raycaster, &camera->position, angle, &hit);
        if (distance == 0) {
            distance = raycaster->size.w + raycaster->size.h;
        }
        SDL_RenderLine(renderer, camera->position.x, camera->position.y,
                           camera->position.x + cosf(angle * (M_PI / 180.0f)) * distance,
                           camera->position.y + sinf(angle * (M_PI / 180.0f)) * distance);
    }
}

/**
 * @brief Cast a ray from a point at a given angle and return the distance to the first non-black pixel.
 *
 * This function simulates raycasting by moving step-by-step from the starting point in the specified direction
 * until it hits a non-black pixel or goes out of bounds. It returns the distance traveled
 * from the starting point to the hit point.
 *
 * @param raycaster The Raycaster instance containing the map.
 * @param point The starting point of the ray.
 * @param angle The angle of the ray in degrees.
 * @param hitColor Pointer to store the color of the hit pixel (if any).
 *
 * @return The distance to the first non-black pixel, or 0 if no hit is found.
 */
float raycast_cast(Raycaster *raycaster, const RaycastPoint *point, float angle, RaycastColor *hitColor) {
    RaycastPoint current = *point;
    while (current.x >= 0 && current.x < raycaster->size.w &&
           current.y >= 0 && current.y < raycaster->size.h) {
        int mapX = (int) current.x;
        int mapY = (int) current.y;
        if (raycaster->map[mapY * raycaster->size.w + mapX] != -1) {
            float dx = current.x - point->x;
            float dy = current.y - point->y;
            *hitColor = raycaster->map[mapY * raycaster->size.w + mapX];
            return sqrtf(dx * dx + dy * dy);
        }
        current.x += cosf(angle * (M_PI / 180.0f));
        current.y += sinf(angle * (M_PI / 180.0f));
    }
    return 0.0f;
}

bool raycast_collides(Raycaster *raycaster, const RaycastPoint *point) {
    if (point->x < 0 || point->x >= raycaster->size.w ||
        point->y < 0 || point->y >= raycaster->size.h) {
        return true;
    }
    int mapX = (int) point->x;
    int mapY = (int) point->y;
    if (raycaster->map[mapY * raycaster->size.w + mapX] != -1) {
        return true;
    }
    return false;
}

void raycast_set_draw_color(SDL_Renderer *renderer, const RaycastColor *color) {
    int32_t c = *color;
    SDL_SetRenderDrawColor(renderer,
                           (c >> 16) & 0xFF,
                           (c >> 8) & 0xFF,
                           c & 0xFF,
                           (c >> 24) & 0xFF);
}
