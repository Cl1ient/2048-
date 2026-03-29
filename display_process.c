#include "common.h"

int pipe_in; // descripteur de lecture du pipe anonyme
int player_num; // numéro du joueur
int first_display = 1; // flag pour le premier affichage
GameState state; // mémoire locale de l'état du jeu
volatile sig_atomic_t refresh_flag = 0; // flag async-signal-safe

// Synchro par signal entre les processus
// Handler déclenché par le signal SIGUSR1
void handle_refresh(int sig) {
    (void)sig; // on ignore le num du signal car c'est que c'est SIGUSR1
    refresh_flag = 1; // on lève juste le drapeau (async-signal-safe)
}

// Affiche la grille (appelé depuis le main, pas depuis le handler)
void display_game() {
    if (read(pipe_in, &state, sizeof(GameState)) > 0) {
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
        else if (state.status == 2) printf("Plus de mouvements possibles \n");
        else if (state.status == 3) printf("Partie abandoné\n");
        else printf("Commandes : z (Haut), s (Bas), q (Gauche), d (Droit), a (Abandonner)\n");
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;
    pipe_in =  atoi(argv[1]);
    player_num = atoi(argv[2]);
    // config du gestionnaire de signal (SIGUSR1)
    struct sigaction sa;
    sa.sa_handler = handle_refresh;
    sigemptyset(&sa.sa_mask); // masque de signaux vide
    sa.sa_flags = 0; // 0 = default
    sigaction(SIGUSR1, &sa, NULL); //

    while (1) {
        // att le signal de refresh (on vérifie le flag AVANT pause()
        // pour ne pas rater un signal arrivé pendant display_game())
        while (!refresh_flag) {
            pause();
        }
        refresh_flag = 0;
        display_game(); // affichage hors du handler (safe)

        if (state.status != 0) {
            sleep(1); // attendre pour bien afficher le messge de fin
            break;
        }
    }
    return 0;
}