#include "unity.h"

#include "raycast.h"

#include <stdlib.h>

#define INIT(w, h) raycaster = raycast_init(w, h)

Raycaster *raycaster = NULL;

void setUp(void) { }

void tearDown(void) {
    if (raycaster) {
        if (raycaster->map) {
            free(raycaster->map);
        }
        free(raycaster);
    }
    raycaster = NULL;
}

void test_raycast_init(void) {
    int w = 100;
    int h = 50;
    raycaster = raycast_init(w, h);
    TEST_ASSERT_NOT_NULL(raycaster);
    TEST_ASSERT_NOT_NULL(raycaster->map);
    TEST_ASSERT_EQUAL_INT(w, raycaster->width);
    TEST_ASSERT_EQUAL_INT(h, raycaster->height);
}

void test_raycast_draw(void) {
    INIT(100, 50);
    RaycastRect rect = {10, 5, 20, 10};
    RaycastColor color = 0xFF00FF00;
    raycast_draw(raycaster, &rect, &color);
    for (int i = 0; i < rect.h; i++) {
        for (int j = 0; j < rect.w; j++) {
            int x = rect.x + j;
            int y = rect.y + i;
            TEST_ASSERT_EQUAL_INT(color, raycaster->map[y * raycaster->width + x]);
        }
    }
}

void test_raycast_erase(void) {
    INIT(100, 50);
    RaycastRect rect = {10, 5, 20, 10};
    RaycastColor color = 0xFF00FF00;
    raycast_draw(raycaster, &rect, &color);
    raycast_erase(raycaster, &rect);
    for (int i = 0; i < rect.h; i++) {
        for (int j = 0; j < rect.w; j++) {
            int x = rect.x + j;
            int y = rect.y + i;
            TEST_ASSERT_EQUAL_INT(RAYCAST_EMPTY, raycaster->map[y * raycaster->width + x]);
        }
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_raycast_init);
    RUN_TEST(test_raycast_draw);
    RUN_TEST(test_raycast_erase);
    return UNITY_END();
}

