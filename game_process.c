#include "common.h"
#include "client_manager.h"

int shm_id;
ShmSlot* shm_array; // Tableau dans la mémoire partagée
int N_GAMES;        // Nombre max de parties en parallèle

pthread_mutex_t mutex_shm = PTHREAD_MUTEX_INITIALIZER; // Exclusion mutuelle
sem_t sem_move;
sem_t sem_goal;
int server_running = 1;

pthread_t t_main, t_move, t_goal; // id des threads
unsigned int rand_seed; // graine pour rand_r (thread-safe)

// handler vide pour réveiller le thread principal du pause()
void thread_wakeup(int sig) { (void)sig; }

// handler pour récolter les processus display terminés (évite les zombies)
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0); // récolte tous les fils terminés
}

// Apparition d'une tuile aléatoire (2 ou 4) sur une case vide
// S41 : prend maitenant un pointeur vers la grille du joueur actuel
void add_tile(GameState *current_game) {
    int empty[16][2];
    int count = 0;

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(current_game->grid[i][j] == 0) {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
        }
    }

    if(count > 0) {
        int r = rand_r(&rand_seed) % count;
        int line = empty[r][0];
        int column = empty[r][1];

        if ((rand_r(&rand_seed) % 10) == 0) {
            current_game->grid[line][column] = 4;
        }
        else {
            current_game->grid[line][column] = 2;
        }
    }
}

// Déplace et fusionne n'importe quelle ligne/colonne comme un mouvement vers la gauche
int process_line(int line[4], GameState *current_game) {
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
            current_game->score += result[i]; // maj du score
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
void move_logic(GameState *current_game, Command cmd) {
    int moved = 0;
    int temp_line[4];

    for (int i = 0; i < 4; i++) {
        // "copie" du jeu actuel dans l'ordre souhaité
        for (int j = 0; j < 4; j++) {
            if (cmd == LEFT)        temp_line[j] = current_game->grid[i][j];
            else if (cmd == RIGHT)  temp_line[j] = current_game->grid[i][3-j]; // début par la fin de ligne
            else if (cmd == UP)     temp_line[j] = current_game->grid[j][i];
            else if (cmd == DOWN)   temp_line[j] = current_game->grid[3-j][i]; // début par le fin de colonne
        }

        // déplacement et fusion de toute la ligne / colonne
        if (process_line(temp_line, current_game))
            moved = 1;

        // maj du jeu après déplacement et fusion
        for (int j = 0; j < 4; j++) {
            if (cmd == LEFT)        current_game->grid[i][j] = temp_line[j];
            else if (cmd == RIGHT)  current_game->grid[i][3-j] = temp_line[j];
            else if (cmd == UP)     current_game->grid[j][i] = temp_line[j];
            else if (cmd == DOWN)   current_game->grid[3-j][i] = temp_line[j];
        }
    }

    if (moved) add_tile(current_game);
}

void* thread_move_score(void* arg) {
    (void)arg;
    while(server_running) {
        sem_wait(&sem_move); // att le signal du main thread via sémaphore
        pthread_mutex_lock(&mutex_shm); // Exclusion mutuelle

        for(int i = 0; i < N_GAMES; i++) {
            if(shm_array[i].state_flag == 1) { // 1 = partie à traiter
                move_logic(&shm_array[i].state, shm_array[i].cmd);
                shm_array[i].state_flag = 2; // donne les infos au thread goal
                sem_post(&sem_goal); // Réveille le thread goal
            }
        }
        pthread_mutex_unlock(&mutex_shm);
    }
    return NULL;
}

void* thread_goal(void* arg) {
    (void)arg;
    while(server_running) {
        sem_wait(&sem_goal); // Att le signal de move & score via sémaphore
        pthread_mutex_lock(&mutex_shm);

        for(int i = 0; i < N_GAMES; i++) {
            if(shm_array[i].state_flag == 2) {
                GameState* current_game = &shm_array[i].state;

                // verif victoire (tuile 2048)
                for(int r = 0; r < 4; r++) {
                    for(int c = 0; c < 4; c++) {
                        if(current_game->grid[r][c] == 2048) current_game->status = 1;
                    }
                }

                // verif défaite (aucun coup possible)
                int can_move = 0;
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 4; c++) {
                        if (current_game->grid[r][c] == 0) can_move = 1;
                        if (r < 3 && current_game->grid[r][c] == current_game->grid[r+1][c]) can_move = 1;
                        if (c < 3 && current_game->grid[r][c] == current_game->grid[r][c+1]) can_move = 1;
                    }
                }
                if (!can_move && current_game->status != 1) current_game->status = 2;

                // Synchro avec le processus d'affichage du joueur concerné
                pthread_mutex_lock(&mutex_clients); // Protection du Tas (clients)
                for(int k = 0; k < num_clients; k++) {
                    if(clients[k].client_pid == shm_array[i].client_pid) {
                        *(clients[k].state) = *current_game; // Maj du Tas
                        write(clients[k].pipe_display_fd, current_game, sizeof(GameState));
                        break;
                    }
                }
                pthread_mutex_unlock(&mutex_clients);
                shm_array[i].state_flag = 0; // Libère la place dans la SHM
            }
        }
        pthread_mutex_unlock(&mutex_shm);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Lecture de la taille N en argument
    if (argc < 2) {
        printf("Erreur : Lancez le serveur avec le nombre de parties (ex: ./game 5)\n");
        return 1;
    }
    N_GAMES = atoi(argv[1]);

    // création du pipe nommé
    unlink(NAMED_PIPE);
    mkfifo(NAMED_PIPE, 0666);
    rand_seed = (unsigned int)time(NULL); // graine thread-safe pour rand_r

    // création de la mémoire partagée pour N parties
    shm_id = shmget(IPC_PRIVATE, N_GAMES * sizeof(ShmSlot), IPC_CREAT | 0666);
    shm_array = (ShmSlot*) shmat(shm_id, NULL, 0);
    memset(shm_array, 0, N_GAMES * sizeof(ShmSlot));

    // initialisation des sémaphores
    sem_init(&sem_move, 0, 0);
    sem_init(&sem_goal, 0, 0);

    // config des signaux pour les threads
    t_main = pthread_self();
    struct sigaction sa;
    sa.sa_handler = thread_wakeup;
    sigaction(SIGUSR1, &sa, NULL);

    // handler pour éviter les processus zombies (récolte les fils display)
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART; // SA_RESTART pour ne pas interrompre read()
    sigaction(SIGCHLD, &sa_chld, NULL);

    // lancement des threads
    pthread_create(&t_move, NULL, thread_move_score, NULL);
    pthread_create(&t_goal, NULL, thread_goal, NULL);

    // ouverture du pipe nommé en lecture / écriture
    int fd_in = open(NAMED_PIPE, O_RDWR);
    PlayerInput input; // Nouvelle structure pour différencier les joueurs

    while (server_running) {
        ssize_t n = read(fd_in, &input, sizeof(PlayerInput));

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Erreur read");
            break;
        }

        if (n == 0) continue; // si pipe vide on attend

        // gestion de l'abandon : déconnecte uniquement ce joueur
        if (input.cmd == QUIT) {
            remove_client(input.client_pid);
            continue;
        }

        // Récupère ou crée le joueur
        ClientSession* client = get_or_create_client(&input);

        // Place la commande dans la mémoire partagée pour les threads
        int slot_trouve = 0;
        while (!slot_trouve && server_running) {
            pthread_mutex_lock(&mutex_shm); // Protection

            // Vérifie que ce joueur n'a pas déjà un coup en attente dans la SHM
            // (sinon on copierait l'ancien état du Tas avant que le coup précédent soit appliqué)
            int player_pending = 0;
            for(int i = 0; i < N_GAMES; i++) {
                if (shm_array[i].state_flag != 0 && shm_array[i].client_pid == client->client_pid) {
                    player_pending = 1;
                    break;
                }
            }

            if (!player_pending) {
                for(int i = 0; i < N_GAMES; i++) {
                    if (shm_array[i].state_flag == 0) { // On cherche une place vide
                        shm_array[i].state = *(client->state); // Copie du Tas -> SHM
                        shm_array[i].client_pid = client->client_pid;
                        shm_array[i].cmd = input.cmd;
                        shm_array[i].state_flag = 1;

                        sem_post(&sem_move); // Réveille le thread de calcul
                        slot_trouve = 1;
                        break;
                    }
                }
            }

            pthread_mutex_unlock(&mutex_shm);
            if (!slot_trouve) usleep(10000); // si SHM pleine ou coup en attente, on attend un peu
        }
    }

    // nettoyage
    close(fd_in);
    unlink(NAMED_PIPE);

    // Débloquer les threads en attente sur les sémaphores
    sem_post(&sem_move);
    sem_post(&sem_goal);
    pthread_join(t_move, NULL);
    pthread_join(t_goal, NULL);

    // Destruction des sémaphores
    sem_destroy(&sem_move);
    sem_destroy(&sem_goal);

    // ferme l'affichage de tous les joueurs et libère le tas
    for(int i = 0; i < num_clients; i++) {
        kill(clients[i].display_pid, SIGTERM);
        close(clients[i].pipe_display_fd); // ferme le pipe anonyme (BUG 5)
        free(clients[i].state);
    }
    free(clients);

    // Destruction de la mémoire partagée
    shmdt(shm_array);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}