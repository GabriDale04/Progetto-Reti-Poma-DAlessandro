#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int** map;
int client_socket;

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

    for (int r = 0; r < 0; r++)
    {
        for (int c = 0; c < 0; c++)
            printItem(map[r][c]);
        
        printf("\n");
    }
}

void sendMessage()
{
    int n;
    char buffer[256];

    printf("Message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    n = write(client_socket, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
}

char* readMessage()
{
    int n;
    char buffer[256];

    bzero(buffer, 256);
    n = read(client_socket, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");

    return buffer;
}

void mainLoop()
{
    while (1)
    {
        sendMessage();
        readMessage();
    }
}

int main(int argc, char *argv[])
{
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
    
    mainLoop();
    close(client_socket);

    return 0;
}