#ifndef PARTIE_H
#define PARTIE_H

// Ce fichier contient toutes les fonctions relatives à la gestion des parties.

/* ********** Includes ********** */

#include "rules.h"
#include "server.h"
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

/* ********** Structures ********** */

// Structure pour les parties.
typedef struct {
  partie_t *partie;       // La partie.
  struct pollfd socks[1]; // Les sockets des joueurs à surveiller.
  list *move[4];          // Les messages de mouvements des joueurs.
  list *bomb[4];          // Les messages de bombes des joueurs.
} mp_t;

/* ********** Fonctions de messages de partie ********** */

/* ********** Fonctions parties ********** */

int start_game(partie_t *partie);
void init_mp(mp_t *mp, partie_t *partie);
int update_mp(mp_t *mp, msg_game_t *mg);

/* ********** Fonctions de messages de partie ********** */

msg_grid_t init_msg_grid(partie_t *partie, board board);
int recv_msg_game(msg_game_t *mg, int sock_mdiff);
int send_game_grid(partie_t *partie, board board);

/* ********** Fonctions de création & gestion de partie ********** */

int init_partie(msg_join_ready_t params, client_t client);
int create_partie(client_t client, msg_join_ready_t params);
int add_joueur(partie_t *partie, client_t client);
int find_partie(int type);
void generate_multicast_adr(char *adr, size_t size);
void generate_multicast_ports(int *port_mdiff, int *port_udp);

/* ********** Fonctions utilitaires ********** */
void free_partie(partie_t *partie);

#endif
