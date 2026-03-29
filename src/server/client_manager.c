#include "client_manager.h"
#include "game_logic.h"

ClientSession* clients = NULL; // Tableau dynamique des joueurs
int num_clients = 0;
pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER; // Protection du tableau clients (Tas)

// Récupère un joueur existant ou en crée un nouveau
ClientSession* get_or_create_client(pid_t pid, const char* tty, unsigned int *seed) {
    pthread_mutex_lock(&mutex_clients); // Protection contre la race condition avec thread_goal

    for(int i = 0; i < num_clients; i++) {
        if(clients[i].client_pid == pid) {
            pthread_mutex_unlock(&mutex_clients);
            return &clients[i];
        }
    }

    // allocation dans le tas
    clients = realloc(clients, (num_clients + 1) * sizeof(ClientSession));
    ClientSession* new_client = &clients[num_clients];
    new_client->client_pid = pid;
    new_client->state = malloc(sizeof(GameState));
    memset(new_client->state, 0, sizeof(GameState));
    strncpy(new_client->tty, tty, sizeof(new_client->tty) - 1);
    new_client->tty[sizeof(new_client->tty) - 1] = '\0';
    add_tile(new_client->state, seed); // première tuile

    // création du pipe Anonyme
    int p[2]; pipe(p);
    new_client->pipe_display_fd = p[1];

    // fork pour créer le processus d'affichage du joueur
    pid_t dpid = fork();
    if(dpid == 0) {
        close(p[1]);
        // Redirige stdout vers le terminal du joueur
        int tty_fd = open(tty, O_WRONLY);
        if (tty_fd >= 0) {
            dup2(tty_fd, STDOUT_FILENO);
            close(tty_fd);
        }
        char fd_str[10]; sprintf(fd_str, "%d", p[0]);
        char num_str[12]; sprintf(num_str, "%d", num_clients + 1);
        execl("./display", "display", fd_str, num_str, NULL); // execute display
        exit(0);
    }
    close(p[0]); // ferme le côté lecture
    new_client->display_pid = dpid;

    usleep(100000); // attendre pour laisser le fils s'init

    // affichage initiale
    write(new_client->pipe_display_fd, new_client->state, sizeof(GameState));
    kill(dpid, SIGUSR1);

    num_clients++;
    printf("[Serveur] Joueur %d connecté (PID: %d)\n", num_clients, pid);
    fflush(stdout); // force l'affichage immédiat
    pthread_mutex_unlock(&mutex_clients);
    return new_client;
}

// Supprime un joueur du tableau clients et nettoie ses ressources
void remove_client(pid_t pid) {
    pthread_mutex_lock(&mutex_clients);
    for(int i = 0; i < num_clients; i++) {
        if(clients[i].client_pid == pid) {
            printf("[Serveur] Joueur %d déconnecté (PID: %d)\n", i + 1, pid);
            fflush(stdout);

            // Envoie l'état "quit" au display avant de le fermer
            clients[i].state->status = 3;
            write(clients[i].pipe_display_fd, clients[i].state, sizeof(GameState));
            kill(clients[i].display_pid, SIGUSR1);
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

// Libère tous les clients (à l'arrêt du serveur)
void cleanup_clients(void) {
    for(int i = 0; i < num_clients; i++) {
        kill(clients[i].display_pid, SIGTERM);
        close(clients[i].pipe_display_fd);
        free(clients[i].state);
    }
    free(clients);
}
