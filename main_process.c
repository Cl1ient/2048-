#include "common.h"

int main() {
    mkfifo(NAMED_PIPE, 0666);

    int fd = open(NAMED_PIPE, O_WRONLY);
    if (fd == -1) {
        perror("main_process : open");
        exit(1);
    }

    char input;

    printf("Commandes : z (Haut), s (Bas), q (Gauche), d (Droit), a(Abandoné)\n");
    while (true) {
        scanf(" %c", &input); // ne pas lire le \n pour gagner en rapidité.

        Command cmd;

        if (input == 'z') cmd = UP;
        else if (input == 's') cmd = DOWN;
        else if (input == 'q') cmd = LEFT;
        else if (input == 'd') cmd = RIGHT;
        else if (input == 'a') cmd = QUIT;
        else continue;

        write(fd, &cmd, sizeof(Command));

        if (cmd == QUIT) break; // Quitte la boucle infini
    }

    close(fd);
    unlink(NAMED_PIPE); // détruit le nom
    return 0;
}