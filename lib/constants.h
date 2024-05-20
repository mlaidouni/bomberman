#ifndef CONSTANTS_H
#define CONSTANTS_H

#define FREQ 100

typedef enum ACT {
  A_NORTH,
  A_EAST,
  A_SOUTH,
  A_WEST,
  A_BOMB,
  A_NONE,
  A_QUIT,
  A_TCHAT
} ACT;

#define EMPTY_TILE 0
#define INDEST_WALL_TILE 1
#define DEST_WALL_TILE 2
#define BOMB_TILE 3

#endif
