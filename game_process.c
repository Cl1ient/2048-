#include "common.h"

GameState game;
pthread_t t_main, t_move, t_goal;
int pipe_anon[2];
pid_t pid_display;


void thread_wakeup(int sig) {}

void* thread_move_score(void* arg) {}

void* thread_goal(void* arg) {}

int main() {
    memset(&game, 0, sizeof(GameState));

    pipe(pipe_anon);
    pid_display = fork();

    t_main = pthread_self();
    signal(SIGUSR1, thread_wakeup);

    pthread_create(&t_move, NULL, thread_move_score, NULL);
    pthread_create(&t_goal, NULL, thread_goal, NULL);

    return 0;
}