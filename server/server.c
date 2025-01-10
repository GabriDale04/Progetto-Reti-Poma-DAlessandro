#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define EMPTY_ITEM 0
#define WALL_ITEM 1
#define PLAYER_ITEM 2
#define APPLE_ITEM 3
#define BANANA_ITEM 5
#define GRAPE_ITEM 7

#define MAX_PLAYERS_COUNT 4
#define MAP_WIDTH 70
#define MAP_HEIGHT 20
#define MAX_COMMAND_LEN 100
#define MAX_ARG_LEN 100
#define MAX_ARG_COUNT 10

struct Player {
    int clientSocket;
    char* name;
    int posX;
    int posY;
    int points;
};

struct Player players[MAX_PLAYERS_COUNT];

int map[MAP_HEIGHT][MAP_WIDTH];

void error(const char* msg) {
    perror(msg);
    exit(1);
}

void initializePlayers(){
    for (int i = 0; i < sizeof(players); i++) {
        players[i].clientSocket = -1;
        players[i].name = "";
        players[i].posX = 0;
        players[i].posY = 0;
        players[i].points = 0;
    }
}

int findPlayerIndex(int clientSocket) { 
    // ritorna l'indice del player cosrrispondente alla socket, -1 se non esiste
    int i = 0;
    while (i < sizeof(players) && players[i].clientSocket != clientSocket) {
        i++;
    }

    if (i < sizeof(players))
        return i;
    
    return -1;
}

void createPlayer(int clientSocket, char* name) {
    if(findPlayerIndex(clientSocket) == -1) {

        int i = 0;
        while (i < sizeof(players) && players[i].name != "") {
            i++;
        }

        if (i < sizeof(players)) {
            players[i].clientSocket = clientSocket;
            players[i].name = name;
            players[i].posX = 45;
            players[i].posY = 10;
            players[i].points = 0;

            map[players[i].posY][players[i].posX] = 2; // fare in modo che player diversi inizino da posizioni diverse
        }
    }
}

void getMapDimension(int clientSocket)
{
    int mapDim[2] = { MAP_WIDTH, MAP_HEIGHT };

    int result = write(clientSocket, &mapDim, sizeof(mapDim));

    if (result < 0)
        error("ERROR writing on socket");
}

void getMapMatrix(int clientSocket)
{
    int map_array[MAP_WIDTH * MAP_HEIGHT];
    int k = 0;
    
    for (int i = 0; i < MAP_HEIGHT; i++) 
    {
        for (int j = 0; j < MAP_WIDTH; j++) 
        {
            map_array[k] = map[i][j];
            k++;
        }
    }

    int result = write(clientSocket, &map, MAP_WIDTH * MAP_HEIGHT * sizeof(int));

    if (result < 0)
        error("ERROR writing on socket");      
}

void eatFruit(int playerIndex, int x, int y) {
    int item = map[y][x];
    
    if(item == APPLE_ITEM){
        map[y][x] = EMPTY_ITEM;
        map[y][x + 1] = EMPTY_ITEM;
        players[playerIndex].points += 1;

    } else if (item == APPLE_ITEM + 1){
        map[y][x] = EMPTY_ITEM;
        map[y][x - 1] = EMPTY_ITEM;
        players[playerIndex].points += 1;

    } else if (item == BANANA_ITEM){
        map[y][x] = EMPTY_ITEM;
        map[y][x + 1] = EMPTY_ITEM;
        players[playerIndex].points += 2;

    } else if (item == BANANA_ITEM + 1){
        map[y][x] = EMPTY_ITEM;
        map[y][x - 1] = EMPTY_ITEM;
        players[playerIndex].points += 2;

    }  else if (item == GRAPE_ITEM){
        map[y][x] = EMPTY_ITEM;
        map[y][x + 1] = EMPTY_ITEM;
        players[playerIndex].points += 3;

    } else if (item == GRAPE_ITEM + 1){
        map[y][x] = EMPTY_ITEM;
        map[y][x - 1] = EMPTY_ITEM;
        players[playerIndex].points += 3;
    }

    //printf("Score: %d\n", players[playerIndex].points);  
}

void moveLeft(int clientSocket) {
    int i = findPlayerIndex(clientSocket);

    if(i != -1) {

        int x = players[i].posX - 1;
        int y = players[i].posY;

        if (x >= 0 && map[y][x] != WALL_ITEM) {
            eatFruit(i, x, y);

            map[y][x] = PLAYER_ITEM;
            map[y][x + 1] = EMPTY_ITEM;
            players[i].posX -= 1;
        }
    }
}

void moveRight(int clientSocket) {
    int i = findPlayerIndex(clientSocket);

    if(i != -1) {

        int x = players[i].posX + 1;
        int y = players[i].posY;

        if (x <= MAP_WIDTH && map[y][x] != WALL_ITEM) {
            eatFruit(i, x, y);

            map[y][x] = PLAYER_ITEM;
            map[y][x - 1] = EMPTY_ITEM;
            players[i].posX += 1;
        }
    }
}

void moveUp(int clientSocket) {
    int i = findPlayerIndex(clientSocket);

    if(i != -1) {

        int x = players[i].posX;
        int y = players[i].posY - 1;

        if (y >= 0 && map[y][x] != WALL_ITEM) {
            eatFruit(i, x, y);

            map[y][x] = PLAYER_ITEM;
            map[y + 1][x] = EMPTY_ITEM;
            players[i].posY -= 1;
        }
    }
}

void moveDown(int clientSocket) {
    int i = findPlayerIndex(clientSocket);

    if(i != -1) {

        int x = players[i].posX;
        int y = players[i].posY + 1;

        if (y <= MAP_HEIGHT && map[y][x] != WALL_ITEM) {
            eatFruit(i, x, y);

            map[y][x] = PLAYER_ITEM;
            map[y - 1][x] = EMPTY_ITEM;
            players[i].posY += 1;
        }
    }
}

void divide_input(const char* input, char* command, char arguments[][MAX_ARG_LEN], int* arg_count) {

    // Trova la prima virgola e la separa dagli argomenti
    const char* separator_pos = strchr(input, ',');
    if (separator_pos == NULL) {
        // Nessun argomento
        strncpy(command, input, MAX_COMMAND_LEN - 1);
        command[MAX_COMMAND_LEN - 1] = '\0'; // Termina la stringa
        *arg_count = 0;
        return;
    }

    // Estrazione comando
    size_t command_len = separator_pos - input;
    strncpy(command, input, command_len);
    command[command_len] = '\0';

    // Estrazione argomenti
    *arg_count = 0;
    const char* start = separator_pos + 1;
    while (*start != '\0' && *arg_count < MAX_ARG_COUNT) {
        const char* end = strchr(start, ',');
        if (end == NULL) {
            end = start + strlen(start);
        }

        size_t arg_len = end - start;
        if (arg_len >= MAX_ARG_LEN) {
            arg_len = MAX_ARG_LEN - 1; // Tronca la stringa se troppo lunga
        }
        strncpy(arguments[*arg_count], start, arg_len);
        arguments[*arg_count][arg_len] = '\0';
        *arg_count += 1;

        if (*end == '\0') {
            break;
        }
        start = end + 1;
    }
}

void parse_command(const char* input, int clientSocket) {
    char command[MAX_COMMAND_LEN];
    char arguments[MAX_ARG_COUNT][MAX_ARG_LEN];
    int arg_count;

    // Divisione dell'input in comando e argomenti
    divide_input(input, command, arguments, &arg_count);

    // Parsing del comando e ricerca di una funzione appropriata per gestirlo
    if (strcmp(command, "moveleft") == 0) {
        int distance = atoi(arguments[0]);
        moveLeft(clientSocket);
    }
    else if (strcmp(command, "moveright") == 0) {
        int distance = atoi(arguments[0]);
        moveRight(clientSocket);
    }
    else if (strcmp(command, "moveup") == 0) {
        int distance = atoi(arguments[0]);
        moveUp(clientSocket);
    }
    else if (strcmp(command, "movedown") == 0) {
        int distance = atoi(arguments[0]);
        moveDown(clientSocket);
    }
    else if (strcmp(command, "getmapdimension") == 0) {
        getMapDimension(clientSocket);
    }
    else if (strcmp(command, "getmapmatrix") == 0) {
        getMapMatrix(clientSocket);
    }
    else if (strcmp(command, "getpoints") == 0) {
        getPoints(clientSocket);
    }
    else {
        printf("Invalid command: %s\n", command);
    }
}

void* clientThreadHandler(void* socket) {
    int* socketPtr = (unsigned int*)socket;
    int clientSocket = *socketPtr;

    createPlayer(clientSocket, "nome");

    printf("Accepted new client\n");

    char buffer[256];
    int result;

    while (1) {
        bzero(buffer, 256);

        result = read(clientSocket, buffer, 255);
        if (result < 0) {
            error("ERROR reading from socket");
        }
        else if (result == 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0; // Rimuovi il \n "newline" se presente
        printf("Here is the message: %s\n", buffer);

        parse_command(buffer, clientSocket);

        // char response[] = "Message recived!";
        // int responseLength = strlen(response);
        // result = write(clientSocket, response, responseLength);
        // if (result < 0) {
        //     error("ERROR writing to socket");
        // }
    }

    close(clientSocket);

    return NULL;
}

void acceptloop(int serverSocket) {
    struct sockaddr_in clientSocket;
    socklen_t clientSocketLength = sizeof(clientSocket);
    pthread_t clientThread;

    while (1) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientSocket, &clientSocketLength);
        if (clientSocket < 0) {
            error("ERROR on accept");
        }
        else {
            pthread_create(&clientThread, NULL, &clientThreadHandler, &clientSocket);
            break;
        }
    }

    while (1)
    {
        int fruits[] = { APPLE_ITEM, BANANA_ITEM, GRAPE_ITEM };

        srand(time(NULL));

        // Genera ogni 10 secondi un po' di frutta
        for (int i = 0; i < 10; i++)
        {
            int x = rand() % MAP_WIDTH;
            int y = rand() % MAP_HEIGHT;

            if (map[y][x] == 0 && x + 1 < MAP_WIDTH && map[y][x + 1] == 0)
            {
                int fruit = rand() % 3;
                map[y][x] = fruits[fruit];
                map[y][x + 1] = fruits[fruit] + 1;
            }
        }

        sleep(10);
    }
}

void createMap() {
    for (int r = 0; r < MAP_HEIGHT; r++)
        for (int c = 0; c < MAP_WIDTH; c++)
            if (r == 0 || r == MAP_HEIGHT - 1 || c == 0 || c == MAP_WIDTH - 1)
                map[r][c] = 1;
            else
                map[r][c] = 0;
}

int main(int argc, char* argv[]) {
    initializePlayers();
    createMap();

    if (argc < 2) {
        error("ERROR, no port provided");
        exit(1);
    }

    int serverSocket, portNumber;
    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        error("ERROR opening socket");
    }

    portNumber = atoi(argv[1]);
    printf("Listening on port %d\n", portNumber);

    bzero((char*)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    listen(serverSocket, 5);

    acceptloop(serverSocket);


    close(serverSocket);

    return 0;
}