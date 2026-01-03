#ifndef INC_2048_COMMON_H
#define INC_2048_COMMON_H

#define GRID_SIZE 4

typedef struct {
    int grid[GRID_SIZE][GRID_SIZE];
    int score;
    int status; // 0, 1, 2 or 3
} GameState;

typedef enum { UP, DOWN, LEFT, RIGHT, QUIT } Command;
