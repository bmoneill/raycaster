#ifndef UTIL_H
#define UTIL_H

#include "raycast/raycast.h"

#define BLACK  0xFF000000
#define WHITE  0xFFFFFFFF
#define RED    0xFFFF0000
#define PURPLE 0xFFFF00FF
#define GREEN  0xFF00FF00
#define BLUE   0xFF0000FF

void handle_keypresses(int*, RaycastCamera*, Raycaster*, int*);

#endif
