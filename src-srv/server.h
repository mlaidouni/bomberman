#ifndef SERVER_H
#define SERVER_H

/* ********** Includes ********** */
#include "../lib/message.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* ********** Structures ********** */

// Structure représentant un client.
struct client_t {
  struct sockaddr_in6 adr; // L'adresse du client.
  socklen_t size;          // La taille de l'adresse du client.
  int sock;                // La socket du client.
} typedef client_t;

// Structure représentant un joueur.
struct joueur_t {
  client_t client; // Le client associé, qui stocke les informations réseaux.
  int id;          // L'id du joueur dans sa partie.
  int team;        // L'équipe du joueur (0 ou 1, 0 par défaut).
} typedef joueur_t;

// Structure représentant une partie.
struct partie_t {
  int type;            // Le type de la partie (0: 4 joueurs, 1: 2 équipes).
  joueur_t joueurs[4]; // Les joueurs de la partie.
  int nb_joueurs;      // Le nombre de joueurs dans la partie.
  int end;             // 0 si la partie est terminée, 1 sinon
  struct sockaddr_in6 g_adr; // L'adresse multicast du groupe.
  struct sockaddr_in6 r_adr; // L'adresse de réception des messages du groupe.
  int port_udp;   // Le port sur lequel le serveur reçoit les messages.
  int port_mdiff; // Le port sur lequel le serveur envoie les messages.
  char adr_mdiff[INET6_ADDRSTRLEN]; // L'adresse de multicastt.
} typedef partie_t;
  
// TODO: gestion des différentes parties
// Structure pour stocker les informations des différentes parties gérées par le
// serveur.
struct parties_t {
  partie_t *parties; // Les parties gérées par le serveur.
  int nb_parties;    // Le nombre de parties gérées par le serveur.
} typedef parties_t;

// Structure pour stocker les informations du serveur.
struct server_t {
  int tcp_sock;            // La socket TCP.
  int tcp_port;            // Le port d'écoute de la socket TCP du serveur.
  struct sockaddr_in6 adr; // L'adresse du serveur.
  parties_t parties;       // Les parties gérées par le serveur.
  client_t *clients;       // Les clients connectés au serveur.
  int nb_clients;          // Le nombre de clients connectés au serveur.
} typedef server_t;

// Structure du multicast
struct msg_multicast_info_t {
  int port_udp;  // Le port UDP sur lequel le serveur attend les messages.
  int port;      // Le port sur lequel le serveur multidiffuse ses messages.
  char addr[12]; // L'adresse IPv6 de multicast (16 octets).
} typedef msg_multicast_info_t;

extern server_t srv;

/* ********** Fonctions server ********** */

void affiche_connexion(struct sockaddr_in6 adrclient);
int create_TCP_connection(int port);
int create_UDP_connection(int port);
int accept_client(client_t *client);
int receive_request();

/* ********** Fonctions utilitaires ********** */

void init_msg_game_data(partie_t partie, msg_game_data_t game_data);

#endif
