#include "rules.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint8_t labyrinth[20][20] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 0},
    {2, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 2, 0, 0, 1, 0},
    {2, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 2, 1, 0, 1, 0},
    {2, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 2, 1, 0, 0, 0},
    {2, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 2, 1, 1, 1, 1},
    {0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 0, 1, 1},
    {0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 0, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 2, 0, 0, 0, 0},
    {0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 1, 1, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 2, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 2, 0, 1, 1, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 2, 0, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0}};
int init_board(board *board, TYPE type) {
  board->type = type;

  // On crée une board par défaut avec des murs destructibles et indescructibles
  memcpy(board->grid, labyrinth, sizeof(labyrinth) * sizeof(uint8_t));

  board->players[0] = (player){0, ALIVE, {0, 0}};
  board->players[1] = (player){0, ALIVE, {WIDTH - 1, HEIGHT - 1}};
  board->players[2] = (player){1, ALIVE, {0, HEIGHT - 1}};
  board->players[3] = (player){1, ALIVE, {WIDTH - 1, 0}};
  board->bombs = init_list();

  return 0;
}

int valid_pos(board board, pos p) {
  return p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT;
}

int action_player(board *board, int player, ACT action) {
  // Rappel: x <= BOMB_TILE signifie x est EMPTY_TILE ou x est BOMB_TILE

  assert(player >= 0 && player < NB_PLAYERS);

  if (board->players[player].status == DEAD) {
    return -1;
  }

  pos p = board->players[player].pos;
  switch (action) {
  case A_NORTH:
    if (p.y > 0) {
      if (board->grid[p.x + (p.y - 1) * WIDTH] == BOMB_TILE ||
          board->grid[p.x + (p.y - 1) * WIDTH] == EMPTY_TILE) {
        p.y--;
        break;
      }
    }
    return -1;
  case A_SOUTH:
    if (p.y < HEIGHT - 1) {
      if (board->grid[p.x + (p.y + 1) * WIDTH] == BOMB_TILE ||
          board->grid[p.x + (p.y + 1) * WIDTH] == EMPTY_TILE) {
        p.y++;
        break;
      }
    }
    return -1;
  case A_EAST:
    if (p.x > 0) {
      if (board->grid[(p.x - 1) + p.y * WIDTH] == BOMB_TILE ||
          board->grid[(p.x - 1) + p.y * WIDTH] == EMPTY_TILE) {
        p.x--;
        break;
      }
    }
    return -1;
  case A_WEST:
    if (p.x < WIDTH - 1) {
      if (board->grid[(p.x + 1) + p.y * WIDTH] == BOMB_TILE ||
          board->grid[(p.x + 1) + p.y * WIDTH] == EMPTY_TILE) {
        p.x++;
        break;
      }
    }
    return -1;
  case A_BOMB:
    if (valid_pos(*board, p) && board->grid[p.x + p.y * WIDTH] == EMPTY_TILE) {
      //printf("Bomb placed at %d %d\n", p.x, p.y);
      board->grid[p.x + p.y * WIDTH] = BOMB_TILE;
      bomb *b = malloc(sizeof(bomb));
      b->pos = p;
      gettimeofday(&(b->timer), NULL);
      add_head(board->bombs, b);
      //printf("Bombes dans la liste: %d\n", length(board->bombs));
    } else
      return -1;
    break;
  default:
    return -1;
  }
  board->players[player].pos = p;
  return 0;
}

bomb *get_bomb_pos_list(board board, pos p) {
  list_elem *n = board.bombs->out;
  while (n != NULL) {
    bomb *b = n->curr;
    if (b->pos.x == p.x && b->pos.y == p.y) {
      return b;
    }
    n = n->next;
  }
  return NULL;
}

int explode_pos(board *board, pos bomb_pos);

/*
 * Endommager la position donnée
 */
int damage_pos(board *board, pos p) {
  if (!valid_pos(*board, p)) {
    return -1;
  }
  if (board->grid[p.x + p.y * WIDTH] == BOMB_TILE) {
    remove_elem(board->bombs, get_bomb_pos_list(*board, p));
    explode_pos(board, p);
    return 0;
  }
  if (board->grid[p.x + p.y * WIDTH] == DEST_WALL_TILE) {
    board->grid[p.x + p.y * WIDTH] = EMPTY_TILE;
    return 1;
  }
  if (board->grid[p.x + p.y * WIDTH] == INDEST_WALL_TILE) {
    return 2;
  }
  for (int i = 0; i < NB_PLAYERS; i++) {
    if (board->players[i].pos.x == p.x && board->players[i].pos.y == p.y) {
      board->players[i].status = DEAD;
    }
  }
  return 0;
}

/*
 * Commencer une explosion à la position donnée
 */
int explode_pos(board *board, pos bomb_pos) {
  if (!valid_pos(*board, bomb_pos) ||
      board->grid[bomb_pos.x + bomb_pos.y * WIDTH] != BOMB_TILE) {
        printf("soucis----------------------------\n");
        return -1;
      }
  board->grid[bomb_pos.x + bomb_pos.y * WIDTH] = EMPTY_TILE;
  damage_pos(board, bomb_pos);

  if (!damage_pos(board, (pos){bomb_pos.x - 1, bomb_pos.y})) {
    damage_pos(board, (pos){bomb_pos.x - 2, bomb_pos.y});
  }

  if (!damage_pos(board, (pos){bomb_pos.x + 1, bomb_pos.y})) {
    damage_pos(board, (pos){bomb_pos.x + 2, bomb_pos.y});
  }

  if (!damage_pos(board, (pos){bomb_pos.x, bomb_pos.y - 1})) {
    damage_pos(board, (pos){bomb_pos.x, bomb_pos.y - 2});
  }

  if (!damage_pos(board, (pos){bomb_pos.x, bomb_pos.y + 1})) {
    damage_pos(board, (pos){bomb_pos.x, bomb_pos.y + 2});
  }

  damage_pos(board, (pos){bomb_pos.x - 1, bomb_pos.y - 1});
  damage_pos(board, (pos){bomb_pos.x + 1, bomb_pos.y - 1});
  damage_pos(board, (pos){bomb_pos.x - 1, bomb_pos.y + 1});
  damage_pos(board, (pos){bomb_pos.x + 1, bomb_pos.y + 1});

  return 0;
}

/*
 * Faire exploser toutes les bombes qui ont été posées il y a plus de 3 secondes
 * Renvoie 1 si toutes les bombes ont explosé, 0 sinon
 */
int explode_bombs(board *board) {
  //printf("appel a explode bombs\n");
  list_elem *n = board->bombs->out;
  while ((n = board->bombs->out) != NULL) {
    printf("bomb while yeah\n");
    bomb *b = n->curr;
    struct timeval now;
    gettimeofday(&now, NULL);
    if ((now.tv_sec - (b->timer).tv_sec) >= 3) {
      int err;
      if((err = remove_tail(board->bombs))) printf("remove_tail failed: %d\n", err);
      explode_pos(board, b->pos);
      printf("Bomb exploded at %d %d\n", b->pos.x, b->pos.y);
      //free(b);
    } else {
      return 0;
    }
  }
  return 1;
}

/* Renvoie -1 si la partie n'est pas terminée
 * Renvoie le numéro de l'équipe gagnante sinon
 */
int winner(board board) {
  int nb_alive = 0;
  int last_team = -1;
  for (int i = 0; i < NB_PLAYERS; i++) {
    if (board.players[i].status == ALIVE) {
      nb_alive++;
      if (last_team == -1) {
        last_team = board.players[i].team;
      } else if (last_team != board.players[i].team) {
        return -1;
      }
    }
  }
  if (nb_alive >= 1) {
    return last_team;
  }
  return -1;
}
