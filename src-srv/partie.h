#ifndef PARTIE_H
#define PARTIE_H

// Ce fichier contient toutes les fonctions relatives à la gestion des parties

/* ********** Includes ********** */
#include "server.h"
#include <bits/pthreadtypes.h>

/* ********** Structures ********** */s

/* ********** Fonctions parties ********** */

int start_game(partie_t *partie);
int init_partie(msg_join_ready_t params, client_t client);
int create_partie(client_t client, msg_join_ready_t params);
int add_joueur(partie_t *partie, client_t client);
int find_partie(int type);
void generate_multicast_adr(char *adr, size_t size);
void generate_multicast_ports(int *port_mdiff, int *port_udp);

#endif
