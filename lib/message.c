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

// TODO: Fixer la fonction suivante
/**
 * Créer un message pour intégrer une partie.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint16_t *ms_integrer(msg_integration params) {
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

  uint16_t *message = malloc(sizeof(uint16_t) * 11);
  message[0] =
      (params.game_type << 3) | (params.player_id << 1) | (params.team_id);
  message[1] = params.port_udp;
  message[2] = params.port_mdiff;
  // FIXME: Comment stocker une adresse (16 octets) dans un uint16_t ?
  message[3] = params.adr_mdiff;

  return message;
}

// TODO : Implémenter la fonction suivante
/**
 * Créer un message pour transmettre un message au tchat.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint32_t ms_tchat(msg_tchat params) {
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

// TODO : Implémenter la fonction suivante
msg_integration mg_integrer(uint64_t message){};
