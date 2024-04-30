#ifndef PARTIE_H
#define PARTIE_H

// Ce fichier contient toutes les fonctions relatives à la gestion des parties

/* ********** Includes ********** */
#include "server.h"

/* ********** Structures ********** */

/* ********** Fonctions parties ********** */

int start_game(partie_t partie);
partie_t create_partie(client_t client, msg_join_ready_t params);
int add_joueur(partie_t partie, client_t client);
int find_partie(int type);
void generate_multicast_adr(char *adr, size_t size);
void generate_multicast_ports(int *port_mdiff, int *port_udp);

#endif
