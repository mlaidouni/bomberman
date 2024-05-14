#ifndef CLIENT_H_
#define CLIENT_H_

/* ********** Includes ********** */
#include "../lib/message.h"
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
  struct ipv6_mreq mreq;   // La structure pour la multidiffusion.
} typedef multicast_client_t;

/* ********** Fonctions client ********** */

int connect_to_server(int port);
int join_game(int sock_client, int game_type);
int ready(int sock_client, int game_type, int player_id, int team_id);
int action(int sock_client, int game_type, int player_id, int team_id, int num,
           int action);

int get_game_data(int sock_client, msg_game_data_t *game_data);
int get_grid(int sock_client, msg_grid_t *grid);
int get_grid_tmp(int sock_client, msg_grid_tmp_t *grid);
int get_end_game(int sock_client, msg_end_game_t *end_game);

/* ********** Fonctions utilitaires ********** */

int send_message(int sock, void *message, size_t size, char *msg_type);
int receive_message(int sock, void *message, size_t size, char *msg_type);

#endif