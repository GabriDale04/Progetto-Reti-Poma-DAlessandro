#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>

#define EMPTY_ITEM 0
#define WALL_ITEM 1
#define PLAYER_ITEM 2
#define APPLE_ITEM 3
#define BANANA_ITEM 5
#define GRAPE_ITEM 7

int client_socket;

int map_width;
int map_height;
int* map;

pthread_mutex_t lock;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void sendCommand(char* message)
{
    int result = write(client_socket, message, strlen(message));

    if (result < 0)
        error("ERROR writing to socket");
}

int readKey() 
{
    struct termios oldt, newt;
    int ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void* readKeyThreadDelegate()
{
    while (1)
    {
        int key = readKey();

        pthread_mutex_unlock(&lock);

        if (key == 'w')
            sendCommand("moveup");
        else if (key == 'a')
            sendCommand("moveleft");
        else if (key == 's')
            sendCommand("movedown");
        else if (key == 'd')
            sendCommand("moveright");

        pthread_mutex_unlock(&lock);

        usleep(50000);
    }
}

void setCursorPosition(int x, int y)
{
    printf("\033[%d;%dH", x, y);
}

void clearScreen()
{
    printf("\033[2J");
    printf("\033[H");
    fflush(stdout);
}

void printItem(int item)
{
    if (item == EMPTY_ITEM)
        printf(" ");
    else if (item == WALL_ITEM)
        printf("\033[47m█\033[0m");
    else if (item == PLAYER_ITEM)
        printf("■");
    else if (item == APPLE_ITEM)
        printf("\033[31m🍎\033[0m");
    else if (item == BANANA_ITEM)
        printf("\033[33m🍌\033[0m");
    else if (item == GRAPE_ITEM)
        printf("\033[35m🍇\033[0m");
}

void printMap()
{
    setCursorPosition(0, 0);

    for (int r = 0; r < map_height; r++)
    {
        for (int c = 0; c < map_width; c++)
        {
            int item = map[r * map_width + c];

            printItem(item);
            
            if (item == APPLE_ITEM || item == BANANA_ITEM || item == GRAPE_ITEM)
                c++;
        }
        
        printf("\n");
    }
}

void getMapDimension()
{
    sendCommand("getmapdimension");

    int* dimension = (int*)malloc(2 * sizeof(int));
    int result = read(client_socket, dimension, 2 * sizeof(int));

    if (result < 0)
    {
        error("ERROR reading from socket");
        free(dimension);
    }

    map_width = dimension[0];
    map_height = dimension[1];

    free(dimension);
}

void getMapMatrix()
{
    sendCommand("getmapmatrix");

    map = (int*)malloc(map_width * map_height * sizeof(int));
    int result = read(client_socket, map, map_width * map_height * sizeof(int));

    printMap();
}

void mainloop()
{
    getMapDimension();

    pthread_t keyThread;
    pthread_create(&keyThread, NULL, readKeyThreadDelegate, NULL);

    while (1)
    {   
        pthread_mutex_lock(&lock);

        getMapMatrix();

        pthread_mutex_unlock(&lock);

        usleep(250000);
    }
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&lock, NULL);
    clearScreen();

    struct sockaddr_in serv_addr;
    struct hostent* server;

    char* server_ip;
    int server_port;
    char* player_name;

    if (argc < 4) 
    {
        fprintf(stderr, "Sintassi del comando:\n\tclient [SERVER_IP] [SERVER_PORT] [PLAYER_NAME]\n");
        exit(0);
    } else 
    {
        server_ip = argv[1];
        server_port = atoi(argv[2]);
        player_name = argv[3];
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
        error("ERROR opening socket");

    server = gethostbyname(server_ip);
    if (server == NULL) 
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(server_port);

    if (connect(client_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    
    mainloop();
    close(client_socket);
    pthread_mutex_destroy(&lock);

    return 0;
}