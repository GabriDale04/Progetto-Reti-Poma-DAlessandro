#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

int clientSocket;
char* playerName;

int map_width;
int map_height;
int* map;

bool inGame = false;

pthread_mutex_t lock;

void error(const char* msg) {
    perror(msg);
    exit(0);
}

void sendCommand(char* message) {
    int result = write(clientSocket, message, strlen(message));

    if (result < 0)
        error("ERROR writing to socket");
}

int readKey() {
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

void* readKeyThreadDelegate() {
    while (1) {
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

void setCursorPosition(int x, int y) {
    printf("\033[%d;%dH", x, y);
    fflush(stdout);
}

void clearScreen() {
    printf("\033[2J");
    printf("\033[H");
    fflush(stdout);
}

void printItem(int item) {
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

void printMap() {
    for (int r = 0; r < map_height; r++) {
        for (int c = 0; c < map_width; c++) {
            int item = map[r * map_width + c];

            printItem(item);

            if (item == APPLE_ITEM || item == BANANA_ITEM || item == GRAPE_ITEM)
                c++;
        }

        printf("\n");
    }
}

void setPlayerName()
{
    char* command = "setplayername,";

    size_t cmdSize = strlen(command) + strlen(playerName) + 1;
    char* newCommand = (char*)malloc(cmdSize);

    strcpy(newCommand, command);
    strcat(newCommand, playerName);
    newCommand[cmdSize - 1] = '\0';

    printf("Resulting string: %s\n", newCommand);

    sendCommand(command);

    free(newCommand);
}

void getMapDimension() {
    sendCommand("getmapdimension");

    int* dimension = (int*)malloc(2 * sizeof(int));
    int result = read(clientSocket, dimension, 2 * sizeof(int));

    if (result < 0) {
        error("ERROR reading from socket");
        free(dimension);
    }
    else
    {
        map_width = dimension[0];
        map_height = dimension[1];

        free(dimension);
    }
}

void getMapMatrix() {
    sendCommand("getmapmatrix");

    map = (int*)malloc(map_width * map_height * sizeof(int));
    int result = read(clientSocket, map, map_width * map_height * sizeof(int));

    if (result < 0)
    {
        error("ERROR reading from socket");
        free(map);
    }
    else
    {
        printMap();
        free(map);
    }
}

void getPoints()
{
    sendCommand("getpoints");

    int points = 0;
    int result = read(clientSocket, &points, sizeof(int));

    if (result < 0)
    {
        error("ERROR reading from socket");
    }
    else
    {
        printf("Punti: %d\n", points);
    }
}

void getTime()
{
    sendCommand("gettime");

    int time = 0;
    int result = read(clientSocket, &time, sizeof(int));

    if (result < 0)
    {
        error("ERROR reading from socket");
    }
    else
    {
        // Non togliere gli spazi: servono per pulire il buffer della console
        printf("Tempo rimanente: %d     \n", time);

        if (time == 0)
            inGame = false;
    }
}

void getStandings()
{
    sendCommand("getstandings");

    char buffer[1024];

    int result = read(clientSocket, &buffer, 1024);

    if (result < 0)
        error("ERROR reading from socket");
    else
    {
        clearScreen();
        printf("%s", buffer);
    }
}

void mainloop(int clientSocket) {
    char buffer[256];
    int result;

    setPlayerName();

    while (!inGame) { // connected waiting for start
        bzero(buffer, 256);
        result = read(clientSocket, buffer, 255);

        if (result < 0) {
            error("ERROR reading from socket");
        }
        else if (result == 0) {
            printf("Client disconnected\n");
        }

        buffer[strcspn(buffer, "\n")] = 0; // Rimuovi il \n "newline" se presente
        printf("Here is the message: %s\n", buffer);
        if (strcmp(buffer, "gamestart")) {
            inGame = true;
        }
    }

    // game start

    clearScreen();
    getMapDimension();

    pthread_t keyThread;
    pthread_create(&keyThread, NULL, readKeyThreadDelegate, NULL);

    while (inGame) { // game loop
        pthread_mutex_lock(&lock);

        setCursorPosition(0, 0);
        getPoints();
        getTime();
        getMapMatrix();

        pthread_mutex_unlock(&lock);

        usleep(250000);
    }

    getStandings();
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&lock, NULL);
    clearScreen();

    struct sockaddr_in serv_addr;
    struct hostent* server;

    char* server_ip;
    int server_port;

    if (argc < 4) {
        fprintf(stderr, "Sintassi del comando:\n\tclient [SERVER_IP] [SERVER_PORT] [PLAYER_NAME]\n");
        exit(0);
    }
    else {
        server_ip = argv[1];
        server_port = atoi(argv[2]);
        playerName = argv[3];
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
        error("ERROR opening socket");

    server = gethostbyname(server_ip);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(server_port);

    if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    else
        printf("Waiting for game to start...\n");

    mainloop(clientSocket);
    close(clientSocket);
    pthread_mutex_destroy(&lock);

    return 0;
}