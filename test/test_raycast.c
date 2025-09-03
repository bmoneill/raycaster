#include "unity.h"

#include "raycast.h"

#include <stdlib.h>

Raycaster *raycaster = NULL;

void setUp(void) { }

void tearDown(void) {
    if (raycaster) {
        if (raycaster->map) {
            free(raycaster->map);
        }
        free(raycaster);
        raycaster = NULL;
    }
}

void test_raycast_init(void) {
    int w = 100;
    int h = 50;
    raycaster = raycast_init(w, h);
    TEST_ASSERT_NOT_NULL(raycaster);
    TEST_ASSERT_NOT_NULL(raycaster->map);
    TEST_ASSERT_EQUAL_INT(w, raycaster->w);
    TEST_ASSERT_EQUAL_INT(h, raycaster->h);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_raycast_init);
    return UNITY_END();
}

