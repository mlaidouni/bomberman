#ifndef MESSAGE_H_
#define MESSAGE_H_

/* ********** Includes ********** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* - ms_ : fonction d'envoie (i.e "message_send_")
 * - mg_ : fonction de réception (i.e "message_mg_")
 * - msg_: structures pour les messages. */

/* ***** Structures *****  */

// Structure pour les messages 'join' et 'ready'.
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
} msg_join_ready_t;

// Structure pour les messages du tchat.
typedef struct {
  int dest;      // Le destinataire du message.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
  int len;       // La longueur du message.
  char *data;    // Le message.
} msg_tchat_t;

// Structure pour le déroulement d'une partie.
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
  int num;       // Le numéro du message.
  int action;    // L'action à effectuer.
} msg_game_t;

// Structure pour l'intégration à une partie.
typedef struct {
  int game_type;  // Le type de partie.
  int player_id;  // L'identifiant attribué au joueur.
  int team_id;    // Le numéro d'équipe attribué au joueur.
  int port_udp;   // Le port UDP sur lequel le serveur attend les messages.
  int port_mdiff; // Le port sur lequel le serveur multidiffuse ses messages.
  uint8_t adr_mdiff[16]; // L'adresse IPv6 de multicast (16 octets).

} msg_game_data_t;

// Structure représentant la grille complète
typedef struct {
  int game_type;   // Le type de partie.
  int codereq;     // Le codereq
  int player_id;   // L'identifiant du joueur.
  int team_id;     // Le numéro d'équipe du joueur.
  int num;         // Le numéro du message.
  int hauteur;     // La hauteur de la grille.
  int largeur;     // La largeur de la grille.
  uint8_t *grille; // La grille.

} msg_grid_t;

// Structure représentant la grille temporaire
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
  int num;       // Le numéro du message.
  int nb_cases;  // Le nombre de cases transmises.
  /* La grille, avec la ligne, la colonne et la valeur de chaque case, les unes
   * à la suite des hautres. Donc il y a sizeof(grille)/3 cases dedans. */
  uint8_t grille[];
} msg_grid_tmp_t;

// Structure représentant la fin de partie
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
} msg_end_game_t;

/* ****** Fonctions d'envoie ****** */

uint16_t ms_join(msg_join_ready_t params);
uint16_t ms_ready(msg_join_ready_t params);
uint32_t ms_game(msg_game_t params);
uint8_t *ms_game_data(msg_game_data_t params);
uint8_t *ms_game_grid(msg_grid_t params);
uint8_t *ms_grid_tmp(msg_grid_tmp_t params);
uint16_t ms_end_game(msg_end_game_t params);
uint32_t ms_tchat_cli(msg_tchat_t params);
uint32_t ms_tchat_srv(msg_tchat_t params);

/* ****** Fonctions de réception ****** */

msg_join_ready_t mg_join(uint16_t message);
msg_join_ready_t mg_ready(uint16_t message);
msg_game_t mg_game(uint32_t message);
msg_game_data_t mg_game_data(uint8_t *message);
msg_grid_t mg_game_grid(uint8_t *message);
msg_grid_tmp_t mg_grid_tmp(uint8_t *message);
msg_end_game_t mg_end_game(uint16_t message);
msg_tchat_t mg_tchat(uint8_t *message);

#endif
