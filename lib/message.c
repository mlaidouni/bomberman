#include "message.h"
#include <stdint.h>

/* ************************ Fonctions d'envoie ************************ */

/**
 * Créer un message qui indique le joueur veut rejoindre une partie.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint16_t ms_join(msg_join_ready params) {
  /* CODEREQ = 1 si la partie est en mode 4 joueurs, 2 pour le mode équipe.
   * ID et EQ sont ignorés */
  uint16_t message = (params.game_type << 3) | (0 << 1) | 0;
  return message;
}

/**
 * Créer un message qui indique le joueur est prêt.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint16_t ms_ready(msg_join_ready params) {
  /* CODEREQ = 3 si la partie est en mode 4 joueurs, 3 pour le mode équipe.
   * ID est une valeur entre 0 et 3 correspondant à l'identifiant du joueur.
   * EQ vaut 0 ou 1 et correspond à l'identifiant de l'équipe du joueur si
   * CODEREQ vaut 2. EQ est ignoré sinon. */
  uint16_t message =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  return message;
}

/**
 * Créer un message pour transmettre une action de jeu.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint32_t ms_game(msg_game params) {
  /* - CODEREQ = 5 si la partie est en mode 4 joueurs, 6 pour le mode équipe.
   * - ID est l'identifiant du joueur.
   * - EQ est l'identifiant de l'équipe si CODEREQ vaut 6. Ignoré sinon.
   * - NUM est le numéro du message modulo 2^13.
   * - ACTION est un entier représentant l'action à effectuer:
   * -> 0 pour un déplacement d'une case vers le nord.
   * -> 1 pour un déplacement d'une case vers l'est.
   * -> 2 pour un déplacement d'une case vers le sud.
   * -> 3 pour un déplacement d'une case vers l'ouest.
   * -> 4 pour le dépôt d'une bombe.
   * -> 5 pour annuler la dernière demande de déplacement.
   *  */
  uint32_t message = (params.game_type << 19) | (params.player_id << 17) |
                     (params.team_id << 16) | (params.num << 3) | params.action;
  return message;
}

// TODO : Implémenter la fonction suivante
/**
 * Créer un message pour transmettre un message au tchat.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint32_t ms_tchat_cli(msg_tchat params) {
  // NOTE: Quel type est renvoyé, étant donnée que le message est de longueur
  // variable ?
  /* CODEREQ vaut 7 si le message est destiné à un joueur, 8 pour une équipe.
   * ID est l'id du joueur qui envoie le message.
   * EQ est l'id de l'équipe qui envoie le message, si cela a du sens.
   * LEN est un entier représentant le nb de caractères du texte à transmettre.
   * DATA est le texte à transmettre. */

  /* uint32_t message = (params.dest << 12) | (params.player_id << 10) |
                      (params.team_id << 9) | (params.len << 1);
  // Ajout des données
  for (int i = 0; i < params.len; i++)
      message |= params.data[i] << (8 * i + 1); */

  return 0;
}

/**
 * Créer un message pour intégrer une partie.
 * @param params Les paramètres du message.
 * @attention La fonction effectue un malloc, il faut penser à free.
 * @return Le message créé.
 */
uint8_t *ms_integrer(msg_integration params) {
  /*
   * - CODEREQ = 9 si la partie demandée est en mode 4 joueurs, 10 pour le mode
   * équipe.
   * - ID est l'identifiant attribué au joueur. Il vaut entre 0 et 3.
   * - EQ est le numéro d'équipe attribué au joueur. Il vaut 0 ou 1.
   * - PORTUDP est le numéro de port sur lequel le serveur attend les messages
   * UDP des joueurs de la partie.
   * - PORTMDIFF est le numéro de port sur lequel le serveur multidiffuse ses
   * messages aux joueurs de la partie.
   * - ADRMDIFF est l'adresse IPv6 de multicast sous forme d'entier à laquelle
   * les joueurs doivent s'abonner.
   */

  // Le message fait 22 octets
  uint8_t *message = malloc(sizeof(uint8_t) * 22);
  uint16_t header =
      htons((params.game_type << 3) | (params.player_id << 1) | params.team_id);
  memcpy(message, &header, 2); // On copie 2 octets

  // On crée les 2 octets représentant le port UDP
  uint16_t port_udp = htons(params.port_udp);
  memcpy(message + 2, &port_udp, 2);

  // On crée les 2 octets représentant le port de multidiffusion
  uint16_t port_mdiff = htons(params.port_mdiff);
  memcpy(message + 4, &port_mdiff, 2);

  // On copie les 16 octets de l'adresse de multidiffusion
  memcpy(message + 6, params.adr_mdiff, 16);

  return message;
}

// TODO: ms_game_grid(msg_grid params) {}

// TODO: ms_log_grid(msg_log_grid params) {}
uint8_t *ms_grid_tmp(msg_grid_tmp message) {
  /* CODEREQ vaut 12.
   * ID et EQ valent 0.
   * NUM est le numéro du message modulo 2^16. Il numérote les msg multidiffusés
   * toutes les freq ms.
   * NB est le nombre de cases transmises.
   * */
}

// TODO : Implémenter la fonction suivante
/**
 * Créer un message pour transmettre un message au tchat.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint32_t ms_tchat_srv(msg_tchat params) {
  // NOTE: Quel type est renvoyé, étant donnée que le message est de longueur
  // variable ?
  /* CODEREQ vaut 13 si le message est destiné à un joueur, 14 pour une équipe.
   * ID est l'id du joueur qui envoie le message.
   * EQ est l'id de l'équipe qui envoie le message, si cela a du sens.
   * LEN est un entier représentant le nb de caractères du texte à transmettre.
   * DATA est le texte à transmettre. */

  /* uint32_t message = (params.dest << 12) | (params.player_id << 10) |
                      (params.team_id << 9) | (params.len << 1);
  // Ajout des données
  for (int i = 0; i < params.len; i++)
      message |= params.data[i] << (8 * i + 1); */

  return 0;
}

/**
 * Créer un message pour indiquer la fin de la partie.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint16_t ms_end_game(msg_end_game params) {
  /* CODEREQ vaut 15 si la partie est en mode 4 joueurs, 16 pour le mode équipe.
   * ID est l'identifiant du joueur gagant si CODEREQ vaut 15, ignoré sinon.
   * EQ est le numéro de l'équipe gagante si CODEREQ vaut 16, ignoré sinon. */
  uint16_t message =
      (params.game_type << 3) | (params.player_id << 1) | (params.team_id);
  return message;
}

/* ************************ Fonctions de réception ************************ */

/**
 * Extraire le type de partie d'un message 'join'.
 * @param message Le message.
 * @return Le type de partie.
 */
msg_join_ready mg_join(uint16_t message) {
  // NOTE: Est-ce qu'on en renverrait pas plutôt juste le type de partie ?
  msg_join_ready params;
  // On masque tous les bits, sauf les trois de poids faible
  params.game_type = message >> 3;
  return params;
}

/**
 * Extraire toutes les informations d'un message 'ready'.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_join_ready mg_ready(uint16_t message) {
  msg_join_ready params;
  params.game_type = message >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (message >> 1) & 0x3;
  // On masque tous les bits, sauf celui de poids faible
  params.team_id = message & 0x1;
  return params;
}

/**
 * Extraire toutes les informations d'un message de jeu.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_game mg_game(uint32_t message) {
  msg_game params;
  params.game_type = message >> 19;
  params.player_id = (message >> 17) & 0x3;
  params.team_id = (message >> 16) & 0x1;
  params.num = (message >> 3) & 0x1FFF;
  params.action = message & 0x7;
  return params;
}

// TODO : Implémenter la fonction suivante:
msg_tchat mg_tchat(uint32_t message) {}

/**
 * Extraire toutes les informations d'un message d'intégration.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_integration mg_integrer(uint8_t *message) {
  msg_integration params;

  uint16_t header;
  // On copie les 2 premiers octets du message
  memcpy(&header, message, 2);
  // On convertit en Little Endian
  header = ntohs(header);

  params.game_type = header >> 3;
  params.player_id = (header >> 1) & 3;
  params.team_id = header & 1;

  // On récupère le port UDP et le port de multidiffusion
  uint16_t port;
  memcpy(&port, message + 2, 2);
  params.port_udp = ntohs(port);

  memcpy(&port, message + 4, 2);
  params.port_mdiff = ntohs(port);

  // On copie les 16 octets de l'adresse de multidiffusion
  memcpy(params.adr_mdiff, message + 6, 16);

  return params;
}

// TODO : Implémenter la fonction suivante
msg_grid mg_game_grid(uint32_t message) {}

// TODO : Implémenter la fonction suivante
msg_grid_tmp mg_grid_tmp(uint8_t *message) {}

/**
 * Extraire toutes les informations d'un message de fin de partie.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_end_game mg_end_game(uint16_t message) {
  msg_end_game params;
  params.game_type = message >> 3;
  params.player_id = (message >> 1) & 0x3;
  params.team_id = message & 0x1;
  return params;
}
