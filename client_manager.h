#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "common.h"

typedef struct {
    pid_t client_pid;
    GameState* state; // etat du jeu alloué dans le tas
    int pipe_display_fd;   // pipe anonyme dédié à ce joueur
    pid_t display_pid;     // PID du processus d'affichage dédié
    char tty[64];          // terminal du joueur (pour y rediriger stdout du display)
} ClientSession;

extern ClientSession* clients;
extern int num_clients;
extern pthread_mutex_t mutex_clients;

// Récupère un joueur existant ou en crée un nouveau
ClientSession* get_or_create_client(PlayerInput* input);

// Supprime un joueur et nettoie ses ressources
void remove_client(pid_t pid);

#endif
