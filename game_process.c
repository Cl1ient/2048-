#include "common.h"

GameState game;
pthread_t t_main, t_move, t_goal;
int pipe_anon[2];
pid_t pid_display;


void thread_wakeup(int sig) {}

// Apparition d'une tuile aléatoire (2 ou 4) sur une case vide 
void add_tile() {
    int empty[16][2];
    int count = 0;

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(game.grid[i][j] == 0) {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
        }
    }

    if(count > 0) {
        int r = rand() % count;
        int line = empty[r][0];
        int column = empty[r][1];

        if ((rand () % 10) == 0) 
            game.grid[line][column] = 4;
        else
            game.grid[line][column] = 2;
    }
}

void* thread_move_score(void* arg) {}

void* thread_goal(void* arg) {}

int main() {
    memset(&game, 0, sizeof(GameState));

    add_tile();

    pipe(pipe_anon);
    pid_display = fork();

    t_main = pthread_self();
    signal(SIGUSR1, thread_wakeup);

    pthread_create(&t_move, NULL, thread_move_score, NULL);
    pthread_create(&t_goal, NULL, thread_goal, NULL);

    return 0;
}