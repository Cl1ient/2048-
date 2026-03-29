#include "common.h"

int pipe_in; // descripteur de lecture du pipe anonyme
int player_num; // numéro du joueur
int first_display = 1; // flag pour le premier affichage
GameState state; // mémoire locale de l'état du jeu

// Affiche la grille
void display_game() {
    system("clear"); // clear l'affichage de la console
    if (first_display) {
        printf("Connecté au serveur en tant que Joueur %d\n", player_num);
        printf("--------------------------------------------------------------------\n");
        first_display = 0;
    }
    printf("=== JEU 2048 - Joueur %d ===   Score: %d\n\n", player_num, state.score);
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
    if (state.status == 3) printf("Partie abandonée\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;
    pipe_in =  atoi(argv[1]);
    player_num = atoi(argv[2]);

    // boucle simple : read() bloque jusqu'à ce qu'un état arrive
    while (read(pipe_in, &state, sizeof(GameState)) > 0) {
        display_game();
        if (state.status != 0) {
            sleep(1); // attendre pour bien afficher le message de fin
            break;
        }
    }
    return 0;
}