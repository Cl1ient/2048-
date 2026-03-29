#include "common.h"

// vide le buffer d'entrée
void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int main() {
    // ouvre le pipe nommé
    int fd = open(NAMED_PIPE, O_WRONLY);

    // On affiche ce message pour dire au joueur d'ouvrir le jeu
    if (fd == -1) {
        printf("En attente du jeu (lance ./game dans l'autre terminal)...\n");
        while ((fd = open(NAMED_PIPE, O_WRONLY)) == -1) {
            sleep(1);
        }
    }

    char input;
    pid_t pid = getpid(); // S41 on récupère le pid

    printf("Commandes : z (Haut), s (Bas), q (Gauche), d (Droit), a (Abandonner)\n");
    printf("--------------------------------------------------------------------\n");

    while (1) {

        scanf(" %c", &input);
        // seul le premoier caractères compte : zdzdzdzda = z
        vider_buffer();

        Command cmd;
        int valid = 1;

        if (input == 'z') cmd = UP;
        else if (input == 's') cmd = DOWN;
        else if (input == 'q') cmd = LEFT;
        else if (input == 'd') cmd = RIGHT;
        else if (input == 'a') cmd = QUIT;
        else valid = 0;

        if (valid) {
            // crée le paquet qui contient le pid, la command, et le terminal du joueur
            PlayerInput paquet = {pid, cmd, ""};
            char* terminal = ttyname(STDIN_FILENO);
            if (terminal) strncpy(paquet.terminal, terminal, sizeof(paquet.terminal) - 1);
            if (write(fd, &paquet, sizeof(PlayerInput)) == -1) {
                printf("Le serveur de jeu est fermé.\n");
                break;
            }
            if (cmd == QUIT) break;
        } else {
            printf("Touche '%c' ignorée. Utilise z, q, s, d ou a.\n", input);
        }
    }

    close(fd); // on ferme le descripteur de fichier
    return 0;
}