#ifndef CLIENT_H_
#define CLIENT_H_

/* ********** Includes ********** */
#include "../lib/message.h"
#include "../src-srv/rules.h"
#include "../src-srv/server.h"
#include <arpa/inet.h>
#include <ncurses.h>
#include <net/if.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Structure multicast
struct multicast_client_t {
  int sock;                // La socket
  struct sockaddr_in6 adr; // L'adresse
  struct ipv6_mreq grp;    // La structure pour la multidiffusion.
} typedef multicast_client_t;

/* ********** Fonctions client ********** */

int connect_to_server(int port);
int abonnement_mdiff(multicast_client_t *mc, char *adr_mdiff, int port_mdiff);
int action(int sock_client, int game_type, int player_id, int team_id, int num,
           int action);

/* ********** Fonctions client ncurses ********** */

void init_ncurses();

/* ********** Fonctions d'envoi de messages ********** */

int join_game(int sock_client, int game_type);
int ready(int sock_client, int game_type, int player_id, int team_id);

/* ********** Fonctions de réception de messages ********** */

int recv_msg_game_data(msg_game_data_t *game_data, int sock_client);
int recv_msg_game_grid(msg_grid_t *grid, multicast_client_t mc);
int recv_msg_grid_tmp(msg_grid_t *grid, multicast_client_t mc);

/* ********** Fonctions utilitaires ********** */

int send_message(int sock, void *message, size_t size, char *msg_type);

#endif
