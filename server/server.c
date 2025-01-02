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

void printItem(int item)
{
    if (item == 0)
        printf(" ");
    else if (item == 1)
        printf("\033[47m█\033[0m");
    else if (item == 2)
        printf("■");
    else if (item == 3)
        printf("\033[31m🍎\033[0m");
    else if (item == 4)
        printf("\033[33m🍌\033[0m");
    else if (item == 5)
        printf("\033[35m🍇\033[0m");
}

int main() {
    return 0;
}