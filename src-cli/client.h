#ifndef CLIENT_H_
#define CLIENT_H_

/* ********** Includes ********** */
#include "../lib/message.h"
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

/* ********** Fonctions client ********** */

int connect_to_server(int port);
int join_game(int sock_client, int game_type);
int ready(int sock_client, int game_type, int player_id, int team_id);
int action(int sock_client, int game_type, int player_id, int team_id, int num,
           int action);

int get_game_data(int sock_client, msg_game_data *game_data);
int get_grid(int sock_client, msg_grid *grid);
int get_grid_tmp(int sock_client, msg_grid_tmp *grid);
int get_end_game(int sock_client, msg_end_game *end_game);

/* ********** Fonctions utilitaires ********** */

int send_message(int sock, void *message, size_t size, char *msg_type);
int receive_message(int sock, void *message, size_t size, char *msg_type);

#endif