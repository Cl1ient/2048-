#include "common.h"

GameState game; // etat global du jeu
Command last_cmd; // dernière commande reçue
pthread_t t_main, t_move, t_goal; // id des threads
int pipe_anon[2]; // pipe anonyme
pid_t pid_display; // PID du processus d'affichage

// handler vide pour réveiller le thread principal du pause()
void thread_wakeup(int sig) { (void)sig; }

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

        if ((rand () % 10) == 0) {
            game.grid[line][column] = 4;
        }
        else {
            game.grid[line][column] = 2;
        }
    }
}

// Déplace et fusionne n'importe quelle ligne/colonne comme un mouvement vers la gauche
int process_line(int line[4]) {
    int changed = 0;
    int result[4] = {0, 0, 0, 0};
    int pos = 0;

    // glissement des tuiles jusqu'à rencontrer un bord ou une autre tuile
    for (int i = 0; i < 4; i++) {
        if (line[i] != 0) result[pos++] = line[i];
    }

    // fusion de deux tuiles adjancentes de même valeur
    for (int i = 0; i < 3; i++) {
        if (result[i] != 0 && result[i] == result[i+1]) {
            result[i] *= 2;
            game.score += result[i];
            result[i+1] = 0;
            changed = 1;
        }
    }

    // re-glissment des tuiles
    pos = 0;
    int final_line[4] = {0};
    for (int i = 0; i < 4; i++) {
        if (result[i] != 0) final_line[pos++] = result[i];
    }

    // maj de la ligne
    for (int i = 0; i < 4; i++) {
        if (line[i] != final_line[i]) {
            line[i] = final_line[i];
            changed = 1;
        }
    }
    return changed;
}


// Gère la logique de déplacement ligne par ligne OU colonne par colonne
void move_logic(Command cmd) {
    int moved = 0;
    int temp_line[4];

    for (int i = 0; i < 4; i++) {
        // "copie" du jeu actuel dans l'ordre souhaité
        for (int j = 0; j < 4; j++) {
            if (cmd == LEFT)        temp_line[j] = game.grid[i][j];
            else if (cmd == RIGHT)  temp_line[j] = game.grid[i][3-j]; // début par la fin de ligne
            else if (cmd == UP)     temp_line[j] = game.grid[j][i];
            else if (cmd == DOWN)   temp_line[j] = game.grid[3-j][i]; // début par le fin de colonne
        }
        
        // déplacement et fusion de toute la ligne / colonne
        if (process_line(temp_line))
            moved = 1; 

        // maj du jeu après déplacement et fusion
        for (int j = 0; j < 4; j++) {
            if (cmd == LEFT)        game.grid[i][j] = temp_line[j];
            else if (cmd == RIGHT)  game.grid[i][3-j] = temp_line[j];
            else if (cmd == UP)     game.grid[j][i] = temp_line[j];
            else if (cmd == DOWN)   game.grid[3-j][i] = temp_line[j];
        }
    }

    if (moved) add_tile();
}

void* thread_move_score(void* arg) {
    (void)arg;
    while(1) {
        pause(); // att le signal du main thread
        move_logic(last_cmd);
        pthread_kill(t_goal, SIGUSR1); // donne les infos au thread goal 
    }
    return NULL;
}

void* thread_goal(void* arg) {
    (void)arg;
    while(1) {
        pause(); // Att le signal de move & score 

        // verif victoire (tuile 2048)
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(game.grid[i][j] == 2048) game.status = 1;
            }
        }

        // verif défaite (aucun coup possible)
        int can_move = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (game.grid[i][j] == 0) can_move = 1;
                if (i < 3 && game.grid[i][j] == game.grid[i+1][j]) can_move = 1;
                if (j < 3 && game.grid[i][j] == game.grid[i][j+1]) can_move = 1;
            }
        }
        if (!can_move) game.status = 2;

        write(pipe_anon[1], &game, sizeof(GameState));
        kill(pid_display, SIGUSR1); // Synchro avec le processus d'affichage

        pthread_kill(t_main, SIGUSR1);
    }
    return NULL; 
}

int main() {
    // création du pipe nommé
    mkfifo(NAMED_PIPE, 0666);
    srand(time(NULL));
    memset(&game, 0, sizeof(GameState));

    add_tile(); // première tuile

    // création du pipe Anonyme
    pipe(pipe_anon);

    // fork pour créer le processus d'affichage
    pid_display = fork();

    if (pid_display == 0) {
        close(pipe_anon[1]);
        char fd_str[10];
        sprintf(fd_str, "%d", pipe_anon[0]);
        execl("./display", "display", fd_str, NULL); // execute display
        exit(0); 
    }

    close(pipe_anon[0]); // ferme le côté lecture
    sleep(1); // attendre pour laisser le fils s'init

    // config des signaux pour les threads
    t_main = pthread_self();
    struct sigaction sa;
    sa.sa_handler = thread_wakeup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    // lancement des threads
    pthread_create(&t_move, NULL, thread_move_score, NULL);
    pthread_create(&t_goal, NULL, thread_goal, NULL);

    // affichage initiale
    write(pipe_anon[1], &game, sizeof(GameState));
    kill(pid_display, SIGUSR1);

    // ouverture du pipe nommé en lecture / écriture
    int fd_in = open(NAMED_PIPE, O_RDWR); 
    while (1) {
        ssize_t n = read(fd_in, &last_cmd, sizeof(Command));

        if (n < 0) {
            if (errno == EINTR) 
                continue; 
            perror("Erreur read");
            break;
        }

        if (n == 0) continue; // si pipe vide on attend

        // gestion de l'abandon
        if (last_cmd == QUIT) {
            game.status = 3;
            write(pipe_anon[1], &game, sizeof(GameState));
            sleep(1); // laisser le temps au handle_refresh de display
            kill(pid_display, SIGUSR1);
            break;
        }
        
        pthread_kill(t_move, SIGUSR1);
        pause(); // attend que goal ai fini son travail
        if (game.status != 0) { // si la partie est gagné ou perdu on sort de la boucle
            break;
        };
    }
    // nettoyage
    close(fd_in);
    close(pipe_anon[1]);
    sleep(1);
    unlink(NAMED_PIPE);
    kill(pid_display, SIGTERM);

    return 0;
}