#include <stdio.h>

void printItem(int item)
{
    if (item == 0)
        printf(" ");
    else if (item == 1)
        printf("■");
    else if (item == 2)
        printf("\033[31m🍎\033[0m");
    else if (item == 3)
        printf("\033[33m🍌\033[0m");
    else if (item == 4)
        printf("\033[35m🍇\033[0m");
}

int main() {
    printf("Hello, I'm your server!\n");
    return 0;
}