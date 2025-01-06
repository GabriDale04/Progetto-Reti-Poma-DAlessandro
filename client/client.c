#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>

int client_socket;

int map_width;
int map_height;
int* map;

char readKey() 
{
    struct termios oldt, newt;
    char ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void setCursorPosition(int x, int y)
{
    printf("\033[%d;%dH", x, y);
}

void clearScreen()
{
    printf("\033[2J");
    printf("\033[H");
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

void printMap()
{
    setCursorPosition(0, 0);

    for (int r = 0; r < map_height; r++)
    {
        for (int c = 0; c < map_width; c++)
            printItem(map[r * map_width + c]);
        
        printf("\n");
    }
}

void sendCommand(char* message)
{
    int result = write(client_socket, message, strlen(message));

    if (result < 0)
        error("ERROR writing to socket");
}

void mainloop()
{
    while (1)
    {
        //sendCommand("getmapmatrix");
        //r();
    }
}

void getMapMatrix()
{
    sendCommand("getmapmatrix");

    map = (int*)malloc(map_width * map_height * sizeof(int));
    int result = read(client_socket, map, map_width * map_height * sizeof(int));

    for (int i = 0; i < map_height; i++) {
        for (int j = 0; j < map_width; j++) {
            printf("%d ", map[i * map_width + j]);
        }
        
        printf("\n");
    }

    printMap();
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

int main(int argc, char *argv[])
{
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
    
    getMapDimension();
    getMapMatrix();
    mainloop();
    close(client_socket);

    return 0;
}