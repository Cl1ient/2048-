#include "common.h"

int pipe_in;
GameState state;

// Synchro par signal entre les processus
void handle_refresh(int sig) {
    (void)sig;
    if (read(pipe_in, &state, sizeof(GameState)) > 0) {
        system("clear"); // clear l'affichage de la console
        printf("=== JEU 2048 ===   Score: %d\n\n", state.score);
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j= 0; j < GRID_SIZE; j++) {
                if (state.grid[i][j] == 0) {
                    printf("[    ] ");
                }else {
                    printf("[%4d] ", state.grid[i][j]);
                }
            }
            printf("\n\n");
        }
        if (state.status == 1) printf("Tu as atteint 2048 \n");
        if (state.status == 2) printf("Plus de mouvements possibles \n");
        if (state.status == 3) printf("Partie abandoné\n");
        fflush(stdout);
    }
}
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    pipe_in =  atoi(argv[1]);

    struct sigaction sa;
    sa.sa_handler = handle_refresh;
    sigemptyset(&sa.sa_mask); // masque de signaux vide
    sa.sa_flags = 0; // 0 = default
    sigaction(SIGUSR1, &sa, NULL); //

    while (1) {
        pause(); // att le signal de refresh
        if (state.status != 0) {
            sleep(1); // attendre pour bien afficher le message de fin
            break;
        }
    }
    return 0;
}