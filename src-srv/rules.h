#ifndef RULES_H
#define RULES_H

#include <time.h>

#include "list.h"

#define WIDTH 20
#define HEIGHT 20

#define NB_PLAYERS 4

typedef enum TYPE { SOLO, TEAM } TYPE;
typedef enum TILE { EMPTY, BOMB, DEST_WALL, INDEST_WALL } TILE;
typedef enum ACTION { A_UP, A_DOWN, A_LEFT, A_RIGHT, A_BOMB } ACTION;
typedef enum PLAYER_STATUS { ALIVE, DEAD } PLAYER_STATUS;

typedef struct pos {
    int x;
    int y;
} pos;

typedef struct bomb {
    pos pos;
    clock_t timer;
} bomb;

/*
    Si on est en mode SOLO, il suffit d'ignorer le champ team
*/
typedef struct player {
    int team;
    PLAYER_STATUS status;
    pos pos;
} player;

typedef struct board {
    TILE grid[WIDTH * HEIGHT];
    TYPE type;
    player players[NB_PLAYERS];
    list *bombs;
} board;


int init_board(board board, TYPE type );
int action_player(board board, int player, ACTION action);
int explode_pos(board board, pos bomb_pos);
int winner(board board);

#endif