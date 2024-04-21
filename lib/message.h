#ifndef MESSAGE_H_
#define MESSAGE_H_

/* - ms_ : fonction d'envoie (i.e "message_send_")
 * - mg_ : fonction de réception (i.e "message_mg_") */

/* ***** Structures *****  */

// Structure pour les messages 'join' et 'ready'.
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
} msg_join_ready;

// Structure pour les messages du tchat.
typedef struct {
  int dest;      // Le destinataire du message.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
  int len;       // La longueur du message.
  char *data;    // Le message.
} msg_tchat;

// Structure pour le déroulement d'une partie.
typedef struct {
  int game_type; // Le type de partie.
  int player_id; // L'identifiant du joueur.
  int team_id;   // Le numéro d'équipe du joueur.
  int num;       // Le numéro du message.
  int action;    // L'action à effectuer.
} msg_game;

// Structure pour l'intégration à une partie.
typedef struct {
  int game_type;  // Le type de partie.
  int player_id;  // L'identifiant attribué au joueur.
  int team_id;    // Le numéro d'équipe attribué au joueur.
  int port_udp;   // Le port UDP sur lequel le serveur attend les messages.
  int port_mdiff; // Le port sur lequel le serveur multidiffuse ses messages.
  uint8_t adr_mdiff[16]; // L'adresse IPv6 de multicast (16 octets).

} msg_integration;

// Structure représentant la grille complète
typedef struct {
  int game_type;    // Le type de partie.
  int player_id;    // L'identifiant du joueur.
  int team_id;      // Le numéro d'équipe du joueur.
  int num;          // Le numéro du message.
  int hauteur;      // La hauteur de la grille.
  int largeur;      // La largeur de la grille.
  uint8_t grille[]; // La grille.
} msg_grid;

/* ****** Fonctions d'envoie ****** */

uint16_t ms_join(msg_join_ready params);
uint16_t ms_ready(msg_join_ready params);
uint32_t ms_game(msg_game params);
uint16_t *ms_integrer(msg_integration params);
uint32_t ms_tchat(msg_tchat params);

/* ****** Fonctions de réception ****** */

msg_join_ready mg_join(uint16_t message);
msg_join_ready mg_ready(uint16_t message);
msg_game mg_game(uint32_t message);

#endif