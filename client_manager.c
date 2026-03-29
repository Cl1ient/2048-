#include "client_manager.h"

ClientSession* clients = NULL; // Tableau dynamique des joueurs
int num_clients = 0;
pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER; // Protection du tableau clients (Tas)

// Déclarations externes depuis game_process.c
extern void add_tile(GameState *current_game);
extern int N_GAMES;

ClientSession* get_or_create_client(PlayerInput* input) {
    pid_t pid = input->client_pid;
    pthread_mutex_lock(&mutex_clients);

    for(int i = 0; i < num_clients; i++) {
        if(clients[i].client_pid == pid) {
            pthread_mutex_unlock(&mutex_clients);
            return &clients[i];
        }
    }

    // Refus de connexion si le serveur est plein
    if (num_clients >= N_GAMES) {
        // Afficher l'erreur dans le terminal du joueur
        FILE* terminal = fopen(input->terminal, "w");
        if (terminal) {
            fprintf(terminal, "[Serveur] Complet ! Max %d partie(s). Connexion refusée.\n", N_GAMES);
            fclose(terminal);
        }
        pthread_mutex_unlock(&mutex_clients);
        return NULL; // signale le refus au serveur
    }

    // allocation dans le tas
    clients = realloc(clients, (num_clients + 1) * sizeof(ClientSession));
    ClientSession* new_client = &clients[num_clients];
    new_client->client_pid = pid;
    new_client->state = malloc(sizeof(GameState));
    memset(new_client->state, 0, sizeof(GameState));
    strncpy(new_client->terminal, input->terminal, sizeof(new_client->terminal) - 1);
    add_tile(new_client->state); // première tuile

    // création du pipe Anonyme
    int p[2]; 
    pipe(p);
    new_client->pipe_display_fd = p[1];

    // fork pour créer le processus d'affichage du joueur
    pid_t dpid = fork();
    if(dpid == 0) {
        close(p[1]);
        char fd_str[10]; 
        sprintf(fd_str, "%d", p[0]);
        char num_str[12]; 
        sprintf(num_str, "%d", num_clients + 1);
        // On passe le chemin du terminal en 3ème argument à display
        execl("./display", "display", fd_str, num_str, new_client->terminal, NULL);
        exit(0);
    }
    close(p[0]); // ferme le côté lecture
    new_client->display_pid = dpid;

    usleep(100000); // attendre pour laisser le fils s'init

    num_clients++;
    printf("[Serveur] Joueur %d connecté (PID: %d)  Terminal: %s\n", num_clients, pid, new_client->terminal);
    fflush(stdout); // force l'affichage immédiat
    pthread_mutex_unlock(&mutex_clients);
    return new_client;
}

void remove_client(pid_t pid) {
    pthread_mutex_lock(&mutex_clients);
    for(int i = 0; i < num_clients; i++) {
        if(clients[i].client_pid == pid) {
            printf("[Serveur] Joueur %d déconnecté (PID: %d)\n", i + 1, pid);
            fflush(stdout);

            // Envoie l'état "quit" au display avant de le fermer
            clients[i].state->status = 3;
            write(clients[i].pipe_display_fd, clients[i].state, sizeof(GameState));
            usleep(50000); // laisse le display afficher le message

            kill(clients[i].display_pid, SIGTERM);
            close(clients[i].pipe_display_fd);
            free(clients[i].state);

            // Décale les éléments suivants pour combler le trou
            for(int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            num_clients--;
            clients = realloc(clients, num_clients * sizeof(ClientSession));
            break;
        }
    }
    pthread_mutex_unlock(&mutex_clients);
}
