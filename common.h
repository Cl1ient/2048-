#ifndef INC_2048_COMMON_H
#define INC_2048_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>


#define GRID_SIZE 4
// le pipe nommé
#define NAMED_PIPE "pipe_2048_input"

typedef struct {
    int grid[GRID_SIZE][GRID_SIZE]; // grille du jeu
    int score; // score actuel
    int status; // 0, 1, 2 or 3
} GameState;

typedef enum { UP, DOWN, LEFT, RIGHT, QUIT } Command; // enum au lieux d'utiliser 1,2,3,4

typedef struct {
    pid_t client_pid;
    Command cmd;
    char tty[64]; // terminal du joueur (ex: /dev/pts/3)
} PlayerInput;

typedef struct {
    GameState state; // la grille à calculer
    pid_t client_pid; // à qui appartient la grille
    Command cmd; // la command à appliquer
    int state_flag; // 0 : Libre, 1 : a bouger, 2 : a verifier
} ShmSlot;

#endif
