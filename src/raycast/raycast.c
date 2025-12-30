#include "raycast.h"

#include <SDL3/SDL_oldnames.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
float raycast_cast(Raycaster* raycaster, float x, float y, float angle, RaycastColor* hitColor) {
    float currentX = x;
    float currentY = y;
    while (currentX >= 0 && currentX < raycaster->width && currentY >= 0
           && currentY < raycaster->height) {
        int mapX = (int) currentX;
        int mapY = (int) currentY;
        if (raycaster->map[mapY * raycaster->width + mapX] != -1) {
            float dx  = currentX - x;
            float dy  = currentY - y;
            *hitColor = raycaster->map[mapY * raycaster->width + mapX];
            return sqrtf(dx * dx + dy * dy);
        }
        currentX += cosf(angle * (M_PI / 180.0f));
        currentY += sinf(angle * (M_PI / 180.0f));
    }
    return 0.0f;
}

/**
 * @brief Cast a ray with texture information.
 *
 * This function performs DDA raycasting to find wall intersections and returns
 * detailed hit information including texture coordinates.
 *
 * @param raycaster The Raycaster instance containing the map.
 * @param x The x coordinate of the starting point.
 * @param y The y coordinate of the starting point.
 * @param angle The angle of the ray in degrees.
 * @param hit Pointer to store the hit information.
 */
void raycast_cast_textured(Raycaster* raycaster, float x, float y, float angle, RaycastHit* hit) {
    float radians    = angle * (M_PI / 180.0f);
    float rayDirX    = cosf(radians);
    float rayDirY    = sinf(radians);

    int   mapX       = (int) x;
    int   mapY       = (int) y;

    float deltaDistX = (rayDirX == 0) ? 1e30f : fabsf(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : fabsf(1.0f / rayDirY);

    int   stepX;
    int   stepY;
    float sideDistX;
    float sideDistY;

    if (rayDirX < 0) {
        stepX     = -1;
        sideDistX = (x - mapX) * deltaDistX;
    } else {
        stepX     = 1;
        sideDistX = (mapX + 1.0f - x) * deltaDistX;
    }

    if (rayDirY < 0) {
        stepY     = -1;
        sideDistY = (y - mapY) * deltaDistY;
    } else {
        stepY     = 1;
        sideDistY = (mapY + 1.0f - y) * deltaDistY;
    }

    // Perform DDA
    int  side     = 0;
    bool hitWall  = false;
    int  maxSteps = raycaster->width + raycaster->height;
    for (int i = 0; i < maxSteps && !hitWall; i++) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        if (mapX < 0 || mapX >= raycaster->width || mapY < 0 || mapY >= raycaster->height) {
            break;
        }

        if (raycaster->map[mapY * raycaster->width + mapX] != RAYCAST_EMPTY) {
            hitWall = true;
        }
    }

    if (!hitWall) {
        hit->distance  = 0.0f;
        hit->wallX     = 0.0f;
        hit->side      = 0;
        hit->textureId = -1;
        return;
    }

    float perpWallDist;
    if (side == 0) {
        perpWallDist = (mapX - x + (1 - stepX) / 2) / rayDirX;
    } else {
        perpWallDist = (mapY - y + (1 - stepY) / 2) / rayDirY;
    }

    float wallX;
    if (side == 0) {
        wallX = y + perpWallDist * rayDirY;
    } else {
        wallX = x + perpWallDist * rayDirX;
    }
    wallX -= floorf(wallX);

    int textureId  = raycaster->map[mapY * raycaster->width + mapX];

    hit->distance  = perpWallDist;
    hit->wallX     = wallX;
    hit->side      = side;
    hit->textureId = textureId;
}

/**
 * @brief Create a new texture.
 *
 * @param width Width of the texture.
 * @param height Height of the texture.
 * @return The newly allocated texture, or NULL on failure.
 */
RaycastTexture* raycast_texture_create(int width, int height) {
    RaycastTexture* texture = (RaycastTexture*) malloc(sizeof(RaycastTexture));
    if (!texture) {
        return NULL;
    }

    texture->pixels = (RaycastColor*) calloc(width * height, sizeof(RaycastColor));
    if (!texture->pixels) {
        free(texture);
        return NULL;
    }

    texture->width  = width;
    texture->height = height;
    return texture;
}

/**
 * @brief Destroy a texture.
 *
 * @param texture The texture to destroy.
 */
void raycast_texture_destroy(RaycastTexture* texture) {
    if (texture) {
        if (texture->pixels) {
            free(texture->pixels);
        }
        free(texture);
    }
}

/**
 * @brief Add a texture to the raycaster.
 *
 * @param raycaster The raycaster instance.
 * @param texture The texture to add.
 */
void raycast_add_texture(Raycaster* raycaster, RaycastTexture* texture) {
    if (!raycaster || !texture) {
        return;
    }

    RaycastTexture** newTextures
        = (RaycastTexture**) realloc(raycaster->textures,
                                     (raycaster->textureCount + 1) * sizeof(RaycastTexture*));

    if (!newTextures) {
        return;
    }

    raycaster->textures                          = newTextures;
    raycaster->textures[raycaster->textureCount] = texture;
    raycaster->textureCount++;
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
bool raycast_collides(Raycaster* raycaster, float x, float y) {
    if (x < 0 || x >= raycaster->width || y < 0 || y >= raycaster->height) {
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
 * @brief Destroy a Raycaster instance.
 *
 * This function frees the memory allocated for the Raycaster instance and its map.
 *
 * @param raycaster The Raycaster instance to destroy.
 */
void raycast_destroy(Raycaster* raycaster) {
    if (raycaster) {
        if (raycaster->map) {
            free(raycaster->map);
        }
        if (raycaster->textures) {
            for (int i = 0; i < raycaster->textureCount; i++) {
                raycast_texture_destroy(raycaster->textures[i]);
            }
            free(raycaster->textures);
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
void raycast_draw(Raycaster* raycaster, const RaycastRect* rect, const RaycastColor* color) {
    for (int i = 0; i < rect->h; i++) {
        for (int j = 0; j < rect->w; j++) {
            if (rect->x + j < 0 || rect->x + j >= raycaster->width || rect->y + i < 0
                || rect->y + i >= raycaster->height) {
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
void raycast_erase(Raycaster* raycaster, const RaycastRect* rect) {
    raycast_draw(raycaster, rect, &RAYCAST_EMPTY);
}

/**
 * @brief Initialize a Raycaster instance.
 *
 * This function allocates a new Raycaster instance and initializes it with the specified width and height.
 *
 * @param w The width of the Raycaster map.
 * @param h The height of the Raycaster map.
 * @return The newly allocated Raycaster instance, or NULL on failure.
 */
Raycaster* raycast_init(int w, int h) {
    Raycaster* raycaster = (Raycaster*) calloc(1, sizeof(Raycaster));
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
int raycast_init_ptr(Raycaster* raycaster, int w, int h) {
    if (raycaster->map) {
        free(raycaster->map);
    }

    raycaster->map = (RaycastColor*) malloc(w * h * sizeof(RaycastColor));
    if (!raycaster->map) {
        return 1;
    }

    raycaster->width        = w;
    raycaster->height       = h;
    raycaster->textures     = NULL;
    raycaster->textureCount = 0;
    return 0;
}

/**
 * @brief Move the camera in the specified direction.
 *
 * @param camera The camera to move.
 * @param direction The direction to move the camera
 */
void raycast_move_camera(RaycastCamera* camera, RaycastDirection direction) {
    float speed = 0.05f;
    if (direction == RAYCAST_FORWARD) {
        camera->posX += camera->dirX * speed;
        camera->posY += camera->dirY * speed;
    } else if (direction == RAYCAST_BACKWARD) {
        camera->posX -= camera->dirX * speed;
        camera->posY -= camera->dirY * speed;
    } else if (direction == RAYCAST_LEFT) {
        camera->posX += camera->dirY * speed;
        camera->posY -= camera->dirX * speed;
    } else if (direction == RAYCAST_RIGHT) {
        camera->posX -= camera->dirY * speed;
        camera->posY += camera->dirX * speed;
    }
}

void raycast_move_camera_with_collision(Raycaster*       raycaster,
                                        RaycastCamera*   camera,
                                        RaycastDirection direction) {
    raycast_move_camera(camera, direction);
    if (raycast_collides(raycaster, camera->posX, camera->posY)) {
        if (direction == RAYCAST_FORWARD) {
            raycast_move_camera(camera, RAYCAST_BACKWARD);
        } else if (direction == RAYCAST_BACKWARD) {
            raycast_move_camera(camera, RAYCAST_FORWARD);
        } else if (direction == RAYCAST_LEFT) {
            raycast_move_camera(camera, RAYCAST_RIGHT);
        } else if (direction == RAYCAST_RIGHT) {
            raycast_move_camera(camera, RAYCAST_LEFT);
        }
    }
}

/**
 * @brief Render the Raycaster map to the display.
 *
 * @param raycaster The Raycaster instance to render.
 * @param camera The camera settings for rendering.
 * @param renderer The SDL_Renderer to use for rendering.
 * @param w The width of the rendering area.
 * @param h The height of the rendering area.
 * @param background The background color to use for empty spaces.
 */
void raycast_render(Raycaster*           raycaster,
                    const RaycastCamera* camera,
                    SDL_Renderer*        renderer,
                    int                  w,
                    int                  h,
                    const RaycastColor*  background) {
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);

    // Render each vertical slice (column) of the screen
    for (int x = 0; x < w; x++) {
        float        angle    = direction - (camera->fov / 2.0f) + (camera->fov * x) / w;
        RaycastColor hitColor = RAYCAST_EMPTY;
        float distance = raycast_cast(raycaster, camera->posX, camera->posY, angle, &hitColor);

        // Simple wall height calculation (inverse proportional to distance)
        int wallHeight = (distance > 0.0f) ? (int) (h / (distance + 0.0001f)) : 0;
        int wallTop    = (h - wallHeight) / 2;
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
 * @brief Render the Raycaster map with textures to the display.
 *
 * @param raycaster The Raycaster instance to render.
 * @param camera The camera settings for rendering.
 * @param renderer The SDL_Renderer to use for rendering.
 * @param w The width of the rendering area.
 * @param h The height of the rendering area.
 * @param background The background color to use for empty spaces.
 */
void raycast_render_textured(Raycaster*           raycaster,
                             const RaycastCamera* camera,
                             SDL_Renderer*        renderer,
                             int                  w,
                             int                  h,
                             const RaycastColor*  background) {
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);

    for (int x = 0; x < w; x++) {
        float      angle = direction - (camera->fov / 2.0f) + (camera->fov * x) / w;
        RaycastHit hit;
        raycast_cast_textured(raycaster, camera->posX, camera->posY, angle, &hit);

        int wallHeight = (hit.distance > 0.0f) ? (int) (h / (hit.distance + 0.0001f)) : 0;
        int wallTop    = (h - wallHeight) / 2;
        int wallBottom = wallTop + wallHeight;

        raycast_set_draw_color(renderer, background);
        SDL_RenderLine(renderer, x, 0, x, wallTop);

        if (hit.textureId >= 0 && hit.textureId < raycaster->textureCount) {
            RaycastTexture* texture = raycaster->textures[hit.textureId];
            int             texX    = (int) (hit.wallX * texture->width);
            if (texX < 0)
                texX = 0;
            if (texX >= texture->width)
                texX = texture->width - 1;

            for (int y = wallTop; y < wallBottom; y++) {
                if (y < 0 || y >= h)
                    continue;

                float texY      = (float) (y - wallTop) / (float) wallHeight;
                int   texYCoord = (int) (texY * texture->height);
                if (texYCoord < 0)
                    texYCoord = 0;
                if (texYCoord >= texture->height)
                    texYCoord = texture->height - 1;

                RaycastColor color = texture->pixels[texYCoord * texture->width + texX];

                if (hit.side == 1) {
                    int r = ((color >> 16) & 0xFF) / 2;
                    int g = ((color >> 8) & 0xFF) / 2;
                    int b = (color & 0xFF) / 2;
                    int a = (color >> 24) & 0xFF;
                    color = (a << 24) | (r << 16) | (g << 8) | b;
                }

                raycast_set_draw_color(renderer, &color);
                SDL_RenderPoint(renderer, x, y);
            }
        } else {
            RaycastColor fallbackColor = (hit.textureId == -1) ? *background : hit.textureId;
            raycast_set_draw_color(renderer, &fallbackColor);
            SDL_RenderLine(renderer, x, wallTop, x, wallBottom);
        }

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
 * @param scale The scale factor for rendering the map.
 * @param background The background color to use for empty spaces.
 * @param wallColor The color to use for textures.
 * @param rayColor The color to use for rendering rays.
 */
void raycast_render_2d(Raycaster*           raycaster,
                       const RaycastCamera* camera,
                       SDL_Renderer*        renderer,
                       int                  w,
                       float                scale,
                       const RaycastColor*  background,
                       const RaycastColor*  wallColor,
                       const RaycastColor*  rayColor) {
    // Render the map
    if (raycaster->textured) {
        for (int y = 0; y < raycaster->height; y++) {
            for (int x = 0; x < raycaster->width; x++) {
                int textureID = raycaster->map[y * raycaster->width + x];
                if (textureID == -1) {
                    raycast_set_draw_color(renderer, background);
                    SDL_FRect rect = { ((float) x) * scale, ((float) y) * scale, scale, scale };
                    SDL_RenderFillRect(renderer, &rect);
                } else {
                    raycast_set_draw_color(renderer, wallColor);
                    SDL_FRect rect = { ((float) x) * scale, ((float) y) * scale, scale, scale };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    } else {
        for (int y = 0; y < raycaster->height; y++) {
            for (int x = 0; x < raycaster->width; x++) {
                RaycastColor color = raycaster->map[y * raycaster->width + x];
                raycast_set_draw_color(renderer, (color == RAYCAST_EMPTY) ? background : &color);
                SDL_RenderPoint(renderer, ((float) x) * scale, ((float) y) * scale);
            }
        }
    }

    // Render the rays
    RaycastColor hit = RAYCAST_EMPTY;
    raycast_set_draw_color(renderer, rayColor);
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);
    float startX    = direction - (camera->fov / 2);
    float endX      = direction + (camera->fov / 2);
    for (float angle = startX; angle <= endX; angle += ((float) camera->fov) / ((float) w)) {
        float distance = raycast_cast(raycaster, camera->posX, camera->posY, angle, &hit);
        if (distance == 0) {
            distance = raycaster->width + raycaster->height;
        }
        SDL_RenderLine(renderer,
                       camera->posX * scale,
                       camera->posY * scale,
                       (camera->posX + cosf(angle * (M_PI / 180.0f)) * distance) * scale,
                       (camera->posY + sinf(angle * (M_PI / 180.0f)) * distance) * scale);
    }
}

/**
 * @brief Rotate the camera by a given angle.
 *
 * @param camera The camera to rotate.
 * @param angle The angle in radians to rotate the camera. Positive values rotate clockwise.
 */
void raycast_rotate_camera(RaycastCamera* camera, float angle) {
    float oldDirX = camera->dirX;
    camera->dirX  = camera->dirX * cosf(angle) - camera->dirY * sinf(angle);
    camera->dirY  = oldDirX * sinf(angle) + camera->dirY * cosf(angle);
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
void raycast_set_draw_color(SDL_Renderer* renderer, const RaycastColor* color) {
#define c ((int32_t) *color)
    SDL_SetRenderDrawColor(renderer, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF);
}

/**
 * @brief Get the version of the libraycast library.
 *
 * @return A string containing the version of the libraycast library.
 */
const char* raycast_version(void) { return LIBRAYCAST_VERSION; }
