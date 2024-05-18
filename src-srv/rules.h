#ifndef RULES_H
#define RULES_H

#include <time.h>

#include "list.h"
#include <arpa/inet.h>

#define WIDTH 20
#define HEIGHT 20

#define EMPTY_TILE 0
#define INDEST_WALL_TILE 1
#define DEST_WALL_TILE 2
#define BOMB_TILE 3

#define NB_PLAYERS 4

typedef enum TYPE { SOLO, TEAM } TYPE;
// typedef enum TILE { EMPTY_TILE, INDEST_WALL_TILE, DEST_WALL_TILE, BOMB_TILE }
// TILE;
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
  uint8_t grid[WIDTH * HEIGHT];
  TYPE type;
  player players[NB_PLAYERS];
  list *bombs;
} board;

int init_board(board board, TYPE type);
int action_player(board board, int player, ACTION action);
int explode_bombs(board boardc);
int winner(board board);

#endif
