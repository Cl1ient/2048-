#ifndef INC_2048_GAME_LOGIC_H
#define INC_2048_GAME_LOGIC_H

#include "common.h"

// Apparition d'une tuile aléatoire (2 ou 4) sur une case vide
void add_tile(GameState *current_game, unsigned int *seed);

// Déplace et fusionne une ligne/colonne comme un mouvement vers la gauche
int process_line(int line[4], GameState *current_game);

// Gère la logique de déplacement ligne par ligne OU colonne par colonne
void move_logic(GameState *current_game, Command cmd, unsigned int *seed);

#endif
