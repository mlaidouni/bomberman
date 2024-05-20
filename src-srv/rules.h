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
typedef enum ACT { A_NORTH, A_EAST, A_SOUTH, A_WEST, A_BOMB, A_NONE, A_QUIT, A_TCHAT } ACT;
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

int init_board(board *board, TYPE type);
int action_player(board *board, int player, ACT action);
int explode_bombs(board *board);
int winner(board board);

#endif
