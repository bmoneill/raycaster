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

    raycaster->width = w;
    raycaster->height = h;
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
    for (int i = 0; i < rect->h; i++) {
        for (int j = 0; j < rect->w; j++) {
            if (rect->x + j < 0 || rect->x + j >= raycaster->width ||
                rect->y + i < 0 || rect->y + i >= raycaster->height) {
                continue;
            }
            raycaster->map[((int) rect->y + i) * raycaster->width + ((int) rect->x + j)] = *color;
        }
    }
}

/**
 * @brief Erase a rectangle area on the Raycaster map.
 *
 * This function sets a rectangle area to be empty.
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
 * @param raycaster The Raycaster instance to render.
 * @param camera The camera settings for rendering.
 * @param w The width of the rendering area.
 * @param h The height of the rendering area.
 * @param renderer The SDL_Renderer to use for rendering.
 * @param background The background color to use for empty spaces.
 */
void raycast_render(Raycaster *raycaster, const RaycastCamera *camera, int w, int h,
                       SDL_Renderer *renderer, const RaycastColor *background) {
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);

    // Render each vertical slice (column) of the screen
    for (int x = 0; x < w; x++) {
        float angle = direction - (camera->fov / 2.0f) + (camera->fov * x) / w;
        RaycastColor hitColor = RAYCAST_EMPTY;
        float distance = raycast_cast(raycaster, camera->posX, camera->posY, angle, &hitColor);

        // Simple wall height calculation (inverse proportional to distance)
        int wallHeight = (distance > 0.0f) ? (int)(h / (distance + 0.0001f)) : 0;
        int wallTop = (h - wallHeight) / 2;
        int wallBottom = wallTop + wallHeight;

        // Draw background above wall
        raycast_set_draw_color(renderer, background);
        SDL_RenderLine(renderer, x, 0, x, wallTop);

        // Draw wall slice
        raycast_set_draw_color(renderer, (hitColor == RAYCAST_EMPTY) ? background : &hitColor);
        SDL_RenderLine(renderer, x, wallTop, x, wallBottom);

        // Draw background below wall
        raycast_set_draw_color(renderer, background);
        SDL_RenderLine(renderer, x, wallBottom, x, h);
    }
}

/**
 * @brief Render the Raycaster map in 2D mode to the display.
 *
 * This function renders the Raycaster map in a 2D view using the provided SDL_Renderer.
 *
 * @param raycaster The Raycaster instance to render.
 * @param camera The camera settings for rendering (currently unused).
 * @param renderer The SDL_Renderer to use for rendering.
 * @param w The width of the rendering area.
 * @param h The height of the rendering area.
 * @param background The background color to use for empty spaces.
 * @param rayColor The color to use for rendering rays.
 */
void raycast_render_2d(Raycaster *raycaster, const RaycastCamera *camera,
                       SDL_Renderer *renderer, int w, int h, const RaycastColor *background,
                       const RaycastColor *rayColor) {
    // Render the map
    for (int y = 0; y < raycaster->height; y++) {
        for (int x = 0; x < raycaster->width; x++) {
            RaycastColor color = raycaster->map[y * raycaster->width + x];
            raycast_set_draw_color(renderer, (color == RAYCAST_EMPTY) ? background : &color);
            SDL_RenderPoint(renderer, x, y);
        }
    }

    // Render the rays
    RaycastColor hit = RAYCAST_EMPTY;
    raycast_set_draw_color(renderer, rayColor);
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);
    float startX = direction - (camera->fov / 2);
    float endX = direction + (camera->fov / 2);
    for (float angle = startX; angle <= endX; angle += (camera->fov * 2) / w) {
        float distance = raycast_cast(raycaster, camera->posX, camera->posY, angle, &hit);
        if (distance == 0) {
            distance = raycaster->width + raycaster->height;
        }
        SDL_RenderLine(renderer, camera->posX, camera->posY,
                           camera->posX + cosf(angle * (M_PI / 180.0f)) * distance,
                           camera->posY + sinf(angle * (M_PI / 180.0f)) * distance);
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
 * @param x The x coordinate of the starting point.
 * @param y The y coordinate of the starting point.
 * @param angle The angle of the ray in degrees.
 * @param hitColor Pointer to store the color of the hit pixel (if any).
 *
 * @return The distance to the first non-black pixel, or 0 if no hit is found.
 */
float raycast_cast(Raycaster *raycaster, float x, float y, float angle, RaycastColor *hitColor) {
    float currentX = x;
    float currentY = y;
    while (currentX >= 0 && currentX < raycaster->width &&
           currentY >= 0 && currentY < raycaster->height) {
        int mapX = (int) currentX;
        int mapY = (int) currentY;
        if (raycaster->map[mapY * raycaster->width + mapX] != -1) {
            float dx = currentX - x;
            float dy = currentY - y;
            *hitColor = raycaster->map[mapY * raycaster->width + mapX];
            return sqrtf(dx * dx + dy * dy);
        }
        currentX += cosf(angle * (M_PI / 180.0f));
        currentY += sinf(angle * (M_PI / 180.0f));
    }
    return 0.0f;
}

/**
 * @brief Check if a point collides with an occupied pixel in the Raycaster map.
 *
 * This function checks if the given point is within the bounds of the Raycaster map and
 * if the corresponding pixel is not empty (i.e., it is occupied).
 *
 * @param raycaster The Raycaster instance containing the map.
 * @param x The x coordinate to check for collision.
 * @param y The y coordinate to check for collision.
 * @return true if the point collides with an occupied pixel, false otherwise.
 */
bool raycast_collides(Raycaster *raycaster, float x, float y) {
    if (x < 0 || x >= raycaster->width ||
        y < 0 || y >= raycaster->height) {
        return true;
    }
    int mapX = (int) x;
    int mapY = (int) y;
    if (raycaster->map[mapY * raycaster->width + mapX] != -1) {
        return true;
    }
    return false;
}

/**
 * @brief Set the SDL_Renderer draw color based on a RaycastColor.
 *
 * This function extracts the ARGB components from the RaycastColor and sets the SDL_Renderer
 * draw color accordingly.
 *
 * @param renderer The SDL_Renderer to set the draw color for.
 * @param color Pointer to the RaycastColor to use for setting the draw color.
 */
void raycast_set_draw_color(SDL_Renderer *renderer, const RaycastColor *color) {
    #define c ((int32_t) *color)
    SDL_SetRenderDrawColor(renderer,
                           (c >> 16) & 0xFF,
                           (c >> 8) & 0xFF,
                           c & 0xFF,
                           (c >> 24) & 0xFF);
}
