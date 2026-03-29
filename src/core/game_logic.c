#include "game_logic.h"

// Apparition d'une tuile aléatoire (2 ou 4) sur une case vide
void add_tile(GameState *current_game, unsigned int *seed) {
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
        int r = rand_r(seed) % count;
        int line = empty[r][0];
        int column = empty[r][1];

        if ((rand_r(seed) % 10) == 0) {
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
void move_logic(GameState *current_game, Command cmd, unsigned int *seed) {
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

    if (moved) add_tile(current_game, seed);
}
