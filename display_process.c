#include "common.h"

int pipe_in;
GameState state;

// Synchro par signal entre les processus
void handle_refresh() {
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
    }
}
int main() {

}