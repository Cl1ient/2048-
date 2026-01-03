#ifndef INC_2048_COMMON_H
#define INC_2048_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


#define GRID_SIZE 4

typedef struct {
    int grid[GRID_SIZE][GRID_SIZE];
    int score;
    int status; // 0, 1, 2 or 3
} GameState;

typedef enum { UP, DOWN, LEFT, RIGHT, QUIT } Command;
