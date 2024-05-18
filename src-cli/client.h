#ifndef CLIENT_H_
#define CLIENT_H_

/* ********** Includes ********** */
#include "../lib/message.h"
#include "../src-srv/rules.h"
#include "../src-srv/server.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

// Structure multicast
struct multicast_client_t {
  int sock;                // La socket
  struct sockaddr_in6 adr; // L'adresse
  struct ipv6_mreq grp;    // La structure pour la multidiffusion.
} typedef multicast_client_t;

/* ********** Fonctions client ********** */

int connect_to_server(int port);
int abonnement_mdiff(multicast_client_t *mc, char *adr_mdiff, int port_mdiff);
int join_game(int sock_client, int game_type);
int ready(int sock_client, int game_type, int player_id, int team_id);
int action(int sock_client, int game_type, int player_id, int team_id, int num,
           int action);

/* ********** Fonctions utilitaires ********** */

int send_message(int sock, void *message, size_t size, char *msg_type);
int recv_message(int sock_client, void *message, size_t msg_size, char *type);
int recv_msg_game_data(msg_game_data_t *game_data, int sock_client);

#endif
