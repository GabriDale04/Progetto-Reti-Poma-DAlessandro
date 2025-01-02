#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH 70
#define MAP_HEIGHT 20

int** map;

void createMap()
{
    map = (int**)calloc(MAP_HEIGHT, sizeof(int*));

    for (int i = 0; i < MAP_HEIGHT; i++)
        map[i] = (int*)calloc(MAP_WIDTH, sizeof(int));
    
    for (int r = 0; r < MAP_HEIGHT; r++)
        for (int c = 0; c < MAP_WIDTH; c++)
            if (r == 0 || r == MAP_HEIGHT - 1 || c == 0 || c == MAP_WIDTH - 1)
                map[r][c] = 1;
}


int main() {
    return 0;
}