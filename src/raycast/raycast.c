#include "raycast.h"

#include <SDL3/SDL_oldnames.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

static int  get_or_add_texture(Raycaster* raycaster, RaycastTexture* texture);
static void render_sprites(Raycaster*           raycaster,
                           const RaycastCamera* camera,
                           SDL_Renderer*        renderer,
                           int                  w,
                           int                  h,
                           float*               zBuffer);

/**
 * @brief Sort order entry used during sprite rendering.
 *
 * Sprites are rendered back-to-front so that closer sprites
 * paint over farther ones correctly.
 */
typedef struct {
    int   index; //!< Index into raycaster->sprites
    float dist; //!< Squared distance from camera (used for sort only)
} SpriteOrder;

static int compare_sprite_order(const void* a, const void* b) {
    float da = ((const SpriteOrder*) a)->dist;
    float db = ((const SpriteOrder*) b)->dist;
    return (da < db) ? 1 : (da > db) ? -1 : 0; /* descending: farthest first */
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
float raycast_cast(Raycaster* raycaster, float x, float y, float angle, int* hitColor) {
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

    texture->pixels = (int*) calloc(width * height, sizeof(int));
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
 * @brief Fill a rectangular region of the floor map with a texture.
 *
 * Registers the texture with the raycaster if it has not been added yet
 * (equivalent to calling raycast_add_texture once), then stamps every cell
 * inside @p rect with that texture's ID.  Cells outside the map bounds are
 * silently ignored.
 *
 * The texture pointer itself is the only thing needed here — it can be
 * created programmatically with raycast_texture_create() or, in the future,
 * loaded from a file with a helper such as raycast_texture_load_file().
 *
 * @param raycaster The raycaster instance.
 * @param texture   Texture to use for the floor in this region.
 * @param rect      Map-space rectangle to fill (x/y/w/h in tile units).
 */
void raycast_add_floor(Raycaster* raycaster, RaycastTexture* texture, const RaycastRect* rect) {
    if (!raycaster || !texture || !rect) {
        return;
    }
    int texId = get_or_add_texture(raycaster, texture);
    if (texId < 0) {
        return;
    }
    for (int row = (int) rect->y; row < (int) (rect->y + rect->h); row++) {
        for (int col = (int) rect->x; col < (int) (rect->x + rect->w); col++) {
            if (col < 0 || col >= raycaster->width || row < 0 || row >= raycaster->height) {
                continue;
            }
            raycaster->floorMap[row * raycaster->width + col] = texId;
        }
    }
}

/**
 * @brief Fill a rectangular region of the ceiling map with a texture.
 *
 * Behaves identically to raycast_add_floor() but writes into the ceiling map.
 *
 * @param raycaster The raycaster instance.
 * @param texture   Texture to use for the ceiling in this region.
 * @param rect      Map-space rectangle to fill (x/y/w/h in tile units).
 */
void raycast_add_ceiling(Raycaster* raycaster, RaycastTexture* texture, const RaycastRect* rect) {
    if (!raycaster || !texture || !rect) {
        return;
    }
    int texId = get_or_add_texture(raycaster, texture);
    if (texId < 0) {
        return;
    }
    for (int row = (int) rect->y; row < (int) (rect->y + rect->h); row++) {
        for (int col = (int) rect->x; col < (int) (rect->x + rect->w); col++) {
            if (col < 0 || col >= raycaster->width || row < 0 || row >= raycaster->height) {
                continue;
            }
            raycaster->ceilMap[row * raycaster->width + col] = texId;
        }
    }
}

/**
 * @brief Create a new sprite.
 *
 * @param x        Initial world-space X position.
 * @param y        Initial world-space Y position.
 * @param texture  Display texture.  May be NULL; can be changed later via
 *                 raycast_sprite_set_texture().  The sprite does NOT take
 *                 ownership of this pointer.
 * @param collides Non-zero if this sprite should block camera movement
 *                 (checked by raycast_collides_sprites()).
 * @return Newly allocated RaycastSprite, or NULL on allocation failure.
 */
RaycastSprite* raycast_sprite_create(float x, float y, RaycastTexture* texture, int collides) {
    RaycastSprite* sprite = (RaycastSprite*) calloc(1, sizeof(RaycastSprite));
    if (!sprite) {
        return NULL;
    }
    sprite->x        = x;
    sprite->y        = y;
    sprite->dirX     = 0.0f;
    sprite->dirY     = 0.0f;
    sprite->texture  = texture;
    sprite->collides = collides;
    return sprite;
}

/**
 * @brief Free a sprite.
 *
 * Only the sprite struct itself is freed.  The texture pointer is not
 * touched — manage texture lifetime separately (e.g. via
 * raycast_add_texture() / raycast_texture_destroy()).
 *
 * @param sprite The sprite to free.
 */
void raycast_sprite_destroy(RaycastSprite* sprite) { free(sprite); }

/**
 * @brief Add a sprite to the raycaster.
 *
 * The raycaster takes ownership of the sprite: it will be freed by
 * raycast_destroy().  The sprite's texture is not affected.
 *
 * @param raycaster The raycaster instance.
 * @param sprite    The sprite to add.
 */
void raycast_add_sprite(Raycaster* raycaster, RaycastSprite* sprite) {
    if (!raycaster || !sprite) {
        return;
    }
    RaycastSprite** newSprites
        = (RaycastSprite**) realloc(raycaster->sprites,
                                    (raycaster->spriteCount + 1) * sizeof(RaycastSprite*));
    if (!newSprites) {
        return;
    }
    raycaster->sprites                           = newSprites;
    raycaster->sprites[raycaster->spriteCount++] = sprite;
}

/**
 * @brief Set the facing direction of a sprite.
 *
 * The direction vector does not need to be normalised.  It is used both
 * by raycast_sprite_move() (the sprite advances along this vector scaled
 * by speed) and by the caller for texture selection (e.g. choosing a
 * front vs. back texture based on dirX/dirY relative to the camera).
 *
 * @param sprite The sprite to update.
 * @param dirX   New X component of the direction vector.
 * @param dirY   New Y component of the direction vector.
 */
void raycast_sprite_set_direction(RaycastSprite* sprite, float dirX, float dirY) {
    if (!sprite) {
        return;
    }
    sprite->dirX = dirX;
    sprite->dirY = dirY;
}

/**
 * @brief Move a sprite along its current direction vector.
 *
 * Advances the sprite's world position by (dirX * speed, dirY * speed).
 * No collision checking is performed here — use raycast_collides() and
 * raycast_collides_sprites() before calling this if you need collision.
 *
 * @param sprite The sprite to move.
 * @param speed  Distance to move (world units).
 */
void raycast_sprite_move(RaycastSprite* sprite, float speed) {
    if (!sprite) {
        return;
    }
    sprite->x += sprite->dirX * speed;
    sprite->y += sprite->dirY * speed;
}

/**
 * @brief Replace a sprite's display texture without changing its position or direction.
 *
 * Use this to implement animations, directional sprites, or any other
 * per-frame texture swap.  The old texture pointer is simply overwritten;
 * neither the old nor the new texture is freed.
 *
 * @param sprite  The sprite to update.
 * @param texture The new display texture (may be NULL to hide the sprite).
 */
void raycast_sprite_set_texture(RaycastSprite* sprite, RaycastTexture* texture) {
    if (!sprite) {
        return;
    }
    sprite->texture = texture;
}

/**
 * @brief Check whether a world position is blocked by a collidable sprite.
 *
 * Returns true if the given point is within RAYCAST_SPRITE_COLLISION_RADIUS
 * world units of any sprite whose collides field is non-zero.
 *
 * @param raycaster The raycaster instance.
 * @param x         World-space X to test.
 * @param y         World-space Y to test.
 * @return true if the position collides with a sprite.
 */
bool raycast_collides_sprites(Raycaster* raycaster, float x, float y) {
    if (!raycaster) {
        return false;
    }
    for (int i = 0; i < raycaster->spriteCount; i++) {
        RaycastSprite* s = raycaster->sprites[i];
        if (!s || !s->collides) {
            continue;
        }
        float dx = x - s->x;
        float dy = y - s->y;
        if (sqrtf(dx * dx + dy * dy) < RAYCAST_SPRITE_COLLISION_RADIUS) {
            return true;
        }
    }
    return false;
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
        if (raycaster->floorMap) {
            free(raycaster->floorMap);
        }
        if (raycaster->ceilMap) {
            free(raycaster->ceilMap);
        }
        if (raycaster->textures) {
            for (int i = 0; i < raycaster->textureCount; i++) {
                raycast_texture_destroy(raycaster->textures[i]);
            }
            free(raycaster->textures);
        }
        if (raycaster->sprites) {
            for (int i = 0; i < raycaster->spriteCount; i++) {
                raycast_sprite_destroy(raycaster->sprites[i]);
            }
            free(raycaster->sprites);
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
void raycast_draw(Raycaster* raycaster, const RaycastRect* rect, const int* color) {
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
    if (raycaster->floorMap) {
        free(raycaster->floorMap);
        raycaster->floorMap = NULL;
    }
    if (raycaster->ceilMap) {
        free(raycaster->ceilMap);
        raycaster->ceilMap = NULL;
    }

    raycaster->map = (int*) malloc(w * h * sizeof(int));
    if (!raycaster->map) {
        return 1;
    }

    raycaster->floorMap = (int*) malloc(w * h * sizeof(int));
    if (!raycaster->floorMap) {
        free(raycaster->map);
        raycaster->map = NULL;
        return 1;
    }

    raycaster->ceilMap = (int*) malloc(w * h * sizeof(int));
    if (!raycaster->ceilMap) {
        free(raycaster->map);
        free(raycaster->floorMap);
        raycaster->map      = NULL;
        raycaster->floorMap = NULL;
        return 1;
    }

    for (int i = 0; i < w * h; i++) {
        raycaster->floorMap[i] = RAYCAST_EMPTY;
        raycaster->ceilMap[i]  = RAYCAST_EMPTY;
    }

    raycaster->width        = w;
    raycaster->height       = h;
    raycaster->textures     = NULL;
    raycaster->textureCount = 0;
    raycaster->sprites      = NULL;
    raycaster->spriteCount  = 0;
    return 0;
}

/**
 * @brief Move the camera in the specified direction.
 *
 * @param camera The camera to move.
 * @param direction The direction to move the camera
 */
void raycast_move_camera(RaycastCamera* camera, RaycastDirection direction, float speed) {
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
                                        RaycastDirection direction,
                                        float            speed) {
    raycast_move_camera(camera, direction, speed);
    if (raycast_collides(raycaster, camera->posX, camera->posY)) {
        if (direction == RAYCAST_FORWARD) {
            raycast_move_camera(camera, RAYCAST_BACKWARD, speed);
        } else if (direction == RAYCAST_BACKWARD) {
            raycast_move_camera(camera, RAYCAST_FORWARD, speed);
        } else if (direction == RAYCAST_LEFT) {
            raycast_move_camera(camera, RAYCAST_RIGHT, speed);
        } else if (direction == RAYCAST_RIGHT) {
            raycast_move_camera(camera, RAYCAST_LEFT, speed);
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
                    const int*           background) {
    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);

    // Render each vertical slice (column) of the screen
    for (int x = 0; x < w; x++) {
        float angle    = direction - (camera->fov / 2.0f) + (camera->fov * x) / w;
        int   hitColor = RAYCAST_EMPTY;
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
 * @brief Render textured floors and ceilings as a full-screen pre-pass.
 *
 * Uses perspective-correct floor casting to project each screen row back into
 * world space, samples the per-cell floor/ceiling texture at that position,
 * and falls back to @p background for cells that have no texture assigned.
 *
 * This function is called internally by raycast_render_textured() before the
 * wall rendering loop so that walls are composited on top.
 *
 * @param raycaster The Raycaster instance.
 * @param camera    The active camera.
 * @param renderer  SDL renderer to draw into.
 * @param w         Render width in pixels.
 * @param h         Render height in pixels.
 * @param background Fallback ARGB color for cells with no floor/ceiling texture.
 */
static void render_floor_ceiling(Raycaster*           raycaster,
                                 const RaycastCamera* camera,
                                 SDL_Renderer*        renderer,
                                 int                  w,
                                 int                  h,
                                 const int*           background) {
    /* The left-most and right-most ray directions for this frame. */
    float rayDirX0 = camera->dirX - camera->planeX;
    float rayDirY0 = camera->dirY - camera->planeY;
    float rayDirX1 = camera->dirX + camera->planeX;
    float rayDirY1 = camera->dirY + camera->planeY;

    int   halfH    = h / 2;

    for (int y = 0; y < h; y++) {
        int isFloor = (y >= halfH);

        /*
         * p is the number of pixels this row is away from the horizon.
         * At the exact horizon (p == 0) the row projects to infinity;
         * fill it with the background colour and move on.
         */
        float p = isFloor ? (float) (y - halfH) : (float) (halfH - y);
        if (p < 1.0f) {
            raycast_set_draw_color(renderer, background);
            SDL_RenderLine(renderer, 0, y, w - 1, y);
            continue;
        }

        /* Perpendicular distance from camera to the floor/ceiling plane. */
        float rowDistance = (0.5f * (float) h) / p;

        /* World-space step between adjacent screen columns on this row. */
        float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / (float) w;
        float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / (float) w;

        /* World-space position of the leftmost pixel on this row. */
        float floorX  = camera->posX + rowDistance * rayDirX0;
        float floorY  = camera->posY + rowDistance * rayDirY0;

        int*  tileMap = isFloor ? raycaster->floorMap : raycaster->ceilMap;

        for (int x = 0; x < w; x++) {
            /* Map cell the current world position falls inside. */
            int cellX = (int) floorf(floorX);
            int cellY = (int) floorf(floorY);

            int texId = RAYCAST_EMPTY;
            if (cellX >= 0 && cellX < raycaster->width && cellY >= 0 && cellY < raycaster->height) {
                texId = tileMap[cellY * raycaster->width + cellX];
            }

            int color;
            if (texId >= 0 && texId < raycaster->textureCount) {
                RaycastTexture* tex = raycaster->textures[texId];
                /* Fractional position within the cell gives texture UVs. */
                float fracX = floorX - floorf(floorX);
                float fracY = floorY - floorf(floorY);
                int   texX  = (int) (fracX * (float) tex->width);
                int   texY  = (int) (fracY * (float) tex->height);
                if (texX < 0)
                    texX = 0;
                if (texX >= tex->width)
                    texX = tex->width - 1;
                if (texY < 0)
                    texY = 0;
                if (texY >= tex->height)
                    texY = tex->height - 1;
                color = tex->pixels[texY * tex->width + texX];
            } else {
                color = *background;
            }

            raycast_set_draw_color(renderer, &color);
            SDL_RenderPoint(renderer, (float) x, (float) y);

            floorX += floorStepX;
            floorY += floorStepY;
        }
    }
}

/**
 * @brief Render all sprites in the raycaster as depth-tested billboards.
 *
 * This is an internal helper called by raycast_render_textured() after the
 * wall pass.  @p zBuffer holds the perpendicular wall distance for each
 * screen column; sprite pixels are only drawn where they are closer than
 * the corresponding wall.  Texture pixels with alpha == 0 are skipped.
 *
 * Sprites are rendered back-to-front (farthest first) so that closer
 * sprites correctly paint over farther ones when they overlap.
 *
 * @param raycaster The Raycaster instance.
 * @param camera    The active camera.
 * @param renderer  SDL renderer to draw into.
 * @param w         Render width in pixels.
 * @param h         Render height in pixels.
 * @param zBuffer   Per-column perpendicular wall distances (length >= w).
 */
static void render_sprites(Raycaster*           raycaster,
                           const RaycastCamera* camera,
                           SDL_Renderer*        renderer,
                           int                  w,
                           int                  h,
                           float*               zBuffer) {
    int n = raycaster->spriteCount;
    if (n == 0)
        return;

    SpriteOrder* order = (SpriteOrder*) malloc((size_t) n * sizeof(SpriteOrder));
    if (!order)
        return;

    for (int i = 0; i < n; i++) {
        float dx       = raycaster->sprites[i]->x - camera->posX;
        float dy       = raycaster->sprites[i]->y - camera->posY;
        order[i].index = i;
        order[i].dist  = dx * dx + dy * dy;
    }
    qsort(order, (size_t) n, sizeof(SpriteOrder), compare_sprite_order);

    /* Inverse camera matrix: transforms world-space offsets into camera space. */
    float invDet = 1.0f / (camera->planeX * camera->dirY - camera->dirX * camera->planeY);

    for (int s = 0; s < n; s++) {
        RaycastSprite* sprite = raycaster->sprites[order[s].index];
        if (!sprite->texture)
            continue;

        float spriteX = sprite->x - camera->posX;
        float spriteY = sprite->y - camera->posY;

        /* Camera-space transform. transformY is the perp. depth of the sprite. */
        float transformX = invDet * (camera->dirY * spriteX - camera->dirX * spriteY);
        float transformY = invDet * (-camera->planeY * spriteX + camera->planeX * spriteY);
        if (transformY <= 0.0f)
            continue; /* behind or on the camera plane */

        int spriteScreenX = (int) ((w / 2.0f) * (1.0f + transformX / transformY));

        /* Sprite occupies the same screen height as a wall at the same depth. */
        int spriteH    = abs((int) ((float) h / transformY));
        int spriteW    = spriteH;

        int drawStartY = -(spriteH / 2) + (h / 2);
        int drawEndY   = (spriteH / 2) + (h / 2);
        int drawStartX = -(spriteW / 2) + spriteScreenX;
        int drawEndX   = (spriteW / 2) + spriteScreenX;

        for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
            if (stripe < 0 || stripe >= w)
                continue;
            /* Skip if a wall (or closer sprite) already owns this column. */
            if (transformY >= zBuffer[stripe])
                continue;

            int texX = (int) ((float) (stripe - drawStartX) / (float) spriteW
                              * (float) sprite->texture->width);
            if (texX < 0)
                texX = 0;
            if (texX >= sprite->texture->width)
                texX = sprite->texture->width - 1;

            for (int y = drawStartY; y < drawEndY; y++) {
                if (y < 0 || y >= h)
                    continue;

                int texY = (int) ((float) (y - drawStartY) / (float) spriteH
                                  * (float) sprite->texture->height);
                if (texY < 0)
                    texY = 0;
                if (texY >= sprite->texture->height)
                    texY = sprite->texture->height - 1;

                int color = sprite->texture->pixels[texY * sprite->texture->width + texX];
                /* Alpha == 0: transparent, skip. */
                if (((color >> 24) & 0xFF) == 0)
                    continue;

                raycast_set_draw_color(renderer, &color);
                SDL_RenderPoint(renderer, (float) stripe, (float) y);
            }
        }
    }

    free(order);
}

/**
 * @brief Render the Raycaster map with textures to the display.
 *
 * Renders textured floors and ceilings first (full-screen pre-pass via
 * render_floor_ceiling()), then draws textured wall slices, then composites
 * sprites on top using a per-column Z-buffer.
 * Cells whose floor/ceiling map entry is RAYCAST_EMPTY fall back to @p background.
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
                             const int*           background) {
    /* Floor/ceiling pre-pass — covers every pixel so the wall loop below
     * only needs to draw the wall slices themselves. */
    render_floor_ceiling(raycaster, camera, renderer, w, h, background);

    /* Per-column perpendicular wall distance for sprite occlusion testing. */
    float* zBuffer = (float*) malloc((size_t) w * sizeof(float));
    if (zBuffer) {
        for (int i = 0; i < w; i++)
            zBuffer[i] = 1e30f;
    }

    float direction = atan2f(camera->dirY, camera->dirX) * (180.0f / M_PI);

    for (int x = 0; x < w; x++) {
        float      angle = direction - (camera->fov / 2.0f) + (camera->fov * x) / w;
        RaycastHit hit;
        raycast_cast_textured(raycaster, camera->posX, camera->posY, angle, &hit);

        /* Record wall depth so sprites can test occlusion later. */
        if (zBuffer) {
            zBuffer[x] = (hit.distance > 0.0f) ? hit.distance : 1e30f;
        }

        int wallHeight = (hit.distance > 0.0f) ? (int) (h / (hit.distance + 0.0001f)) : 0;
        int wallTop    = (h - wallHeight) / 2;
        int wallBottom = wallTop + wallHeight;

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

                int color = texture->pixels[texYCoord * texture->width + texX];

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
            int fallbackColor = (hit.textureId == -1) ? *background : hit.textureId;
            raycast_set_draw_color(renderer, &fallbackColor);
            SDL_RenderLine(renderer, x, wallTop, x, wallBottom);
        }
    }

    /* Sprite pass: composite sprites in front of walls using the Z-buffer. */
    if (zBuffer) {
        render_sprites(raycaster, camera, renderer, w, h, zBuffer);
        free(zBuffer);
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
                       const int*           background,
                       const int*           wallColor,
                       const int*           rayColor) {
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
                int color = raycaster->map[y * raycaster->width + x];
                raycast_set_draw_color(renderer, (color == RAYCAST_EMPTY) ? background : &color);
                SDL_RenderPoint(renderer, ((float) x) * scale, ((float) y) * scale);
            }
        }
    }

    // Render the rays
    int hit = RAYCAST_EMPTY;
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
 * @brief Set the SDL_Renderer draw color based on a int.
 *
 * This function extracts the ARGB components from the int and sets the SDL_Renderer
 * draw color accordingly.
 *
 * @param renderer The SDL_Renderer to set the draw color for.
 * @param color Pointer to the int to use for setting the draw color.
 */
void raycast_set_draw_color(SDL_Renderer* renderer, const int* color) {
#define c ((int32_t) *color)
    SDL_SetRenderDrawColor(renderer, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF);
}

/**
 * @brief Get the version of the libraycast library.
 *
 * @return A string containing the version of the libraycast library.
 */
const char* raycast_version(void) { return RAYCAST_VERSION; }

/**
 * @brief Return the index of a texture already registered with the raycaster,
 *        or add it and return the new index.
 *
 * @param raycaster The raycaster instance.
 * @param texture   The texture to look up or register.
 * @return Index of the texture in raycaster->textures, or -1 on allocation failure.
 */
static int get_or_add_texture(Raycaster* raycaster, RaycastTexture* texture) {
    for (int i = 0; i < raycaster->textureCount; i++) {
        if (raycaster->textures[i] == texture) {
            return i;
        }
    }
    int prevCount = raycaster->textureCount;
    raycast_add_texture(raycaster, texture);
    return (raycaster->textureCount > prevCount) ? prevCount : -1;
}
