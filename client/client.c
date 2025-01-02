#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int** map;

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

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    char buffer[256];

    if (argc < 4) {
        fprintf(stderr, "Sintassi del comando:\n\tclient [SERVER_IP] [SERVER_PORT] [PLAYER_NAME]\n");
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");

    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");

    printf("%s\n", buffer);
    close(sockfd);

    return 0;
}