#include "message.h"
#include <arpa/inet.h>
#include <stdio.h>

/* ************************ Fonctions d'envoie ************************ */

/**
 * Créer un message qui indique le joueur veut rejoindre une partie.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (1 pour le mode 4 joueurs, 2 pour équipe).
 * @param player_id L'identifiant du joueur (valeur ignorée).
 * @param team_id Le numéro de l'équipe du joueur (valeur ignorée).
 * @return Le message créé, au format Big Endian.
 */
uint16_t ms_join(msg_join_ready_t params) {
  /* CODEREQ = 1 si la partie est en mode 4 joueurs, 2 pour le mode équipe.
   * ID et EQ sont ignorés. */
  uint16_t message = (params.game_type << 3) | (0 << 1) | 0;
  // Le message est au format Big Endian
  message = htons(message);
  return message;
}

/**
 * Créer un message qui indique le joueur est prêt.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (3 pour le mode 4 joueurs, 4 pour équipe).
 * @param player_id L'identifiant du joueur (valeur entre 0 et 3).
 * @param team_id Le numéro de l'équipe du joueur (0 ou 1).
 * @return Le message créé, au format Big Endian.
 */
uint16_t ms_ready(msg_join_ready_t params) {
  /* CODEREQ = 3 si la partie est en mode 4 joueurs, 4 pour le mode équipe.
   * ID est une valeur entre 0 et 3 correspondant à l'identifiant du joueur.
   * EQ vaut 0 ou 1 et correspond au numéro de l'équipe du joueur si CODEREQ
   * vaut 2. EQ est ignoré sinon. */
  uint16_t message =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  // Le message est au format Big Endian
  message = htons(message);
  return message;
}

/**
 * Créer un message pour transmettre une action de jeu.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (5 pour le mode 4 joueurs, 6 pour équipe).
 * @param player_id L'identifiant du joueur.
 * @param team_id Le numéro de l'équipe du joueur (ignoré si game_type vaut 5).
 * @param num Le numéro du message modulo 2^13.
 * @param action L'action à effectuer (valeur entre 0 et 5).
 * @return Le message créé, où les deux pairs d'octets sont au format Big
 * Endian.
 */
uint32_t ms_game(msg_game_t params) {
  /* - CODEREQ = 5 si la partie est en mode 4 joueurs, 6 pour le mode équipe.
   * - ID est l'identifiant du joueur.
   * - EQ est le numéro d'équipe si CODEREQ vaut 6. Ignoré sinon.
   * - NUM est le numéro du message modulo 2^13.
   * - ACTION est un entier représentant l'action à effectuer:
   * -> 0 pour un déplacement d'une case vers le nord.
   * -> 1 pour un déplacement d'une case vers l'est.
   * -> 2 pour un déplacement d'une case vers le sud.
   * -> 3 pour un déplacement d'une case vers l'ouest.
   * -> 4 pour le dépôt d'une bombe.
   * -> 5 pour annuler la dernière demande de déplacement.
   *  */

  // On crée les deux premiers octets du message (appelés 'header')
  uint16_t header =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  // Les deux premiers octets sont au format Big Endian
  header = htons(header);

  // On crée les deux derniers octets du message (appelés 'footer')
  uint16_t footer = (params.num << 3) | params.action;
  // Les deux derniers octets sont au format Big Endian
  footer = htons(footer);

  // On met les deux parties du message ensemble
  uint32_t message = ((uint32_t)header << 16) | footer;
  return message;
}

/**
 * Créer un message contenant les informations d'une partie.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (9 pour le mode 4 joueurs, 10 pour
 * équipe).
 * @param player_id L'identifiant attribué au joueur (valeur entre 0 et 3).
 * @param team_id Le numéro d'équipe attribué au joueur (valeur entre 0 et 1).
 * @param port_udp Le numéro de port sur lequel le serveur attend les messages.
 * @param port_mdiff Le numéro de port sur lequel le serveur multidiffuse ses
 * messages.
 * @param adr_mdiff L'adresse IPv6 de multicast.
 * @attention La fonction effectue un malloc, il faut penser à free !
 * @return Le message créé.
 */
uint8_t *ms_game_data(msg_game_data_t params) {
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
  // On crée les deux premiers octets du message (appelés 'header')
  uint16_t header =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  // Les deux premiers octets sont au format Big Endian
  header = htons(header);
  memcpy(message, &header, 2); // On copie 2 octets

  // On crée les 2 octets représentant le port UDP
  uint16_t port_udp = htons(params.port_udp);
  memcpy(message + 2, &port_udp, 2);

  // On crée les 2 octets représentant le port de multidiffusion
  uint16_t port_mdiff = htons(params.port_mdiff);
  memcpy(message + 4, &port_mdiff, 2);

  /* On copie les 16 octets de l'adresse de multidiffusion. L'adresse vient de
   * la structure sockaddr_in, elle est donc déjà au format Big Endian. */
  memcpy(message + 6, params.adr_mdiff, 16);

  return message;
}

/**
 * Créer un message pour transmettre la grille de jeu.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (vaut toujours 11).
 * @param player_id L'identifiant du joueur (vaut toujours 0).
 * @param team_id Le numéro de l'équipe du joueur (vaut toujours 0).
 * @param num Le numéro du message modulo 2^16.
 * @param hauteur La hauteur de la grille.
 * @param largeur La largeur de la grille.
 * @param grille Les cases de la grille de jeu.
 */
uint8_t *ms_game_grid(msg_grid_t params) {
  /*
   * - CODEREQ vaut 11.
   * - ID et EQ valent 0
   * - NUM est le numéro du message modulo 2^16. Numérote les messages
   * multidiffusés toutes les secondes.
   * - HAUTEUR et LARGUEUR sont la hauteur et la largeur de la grille.
   * - Chaque case de la grille est un octet, et peut prendre les valeurs:
   * -> 0 si la case est vide.
   * -> 1 si la case est un mur indesctructible.
   * -> 2 si la case est un mur destructible.
   * -> 3 si la case est une bombe.dsdsfs
   * -> 4 si la case est explosée par une bombe.
   * -> 5 + i si la case contient le joueur
   */

  // Affichage hauteur et largeur
  printf("ms: Hauteur: %d\n", params.hauteur);
  printf("ms: Largeur: %d\n", params.largeur);

  uint8_t *message =
      malloc(sizeof(uint8_t) * (6 + params.hauteur * params.largeur));

  // On crée les deux premiers octets du message (appelés 'header')
  uint16_t header =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  // Les deux premiers octets sont au format Big Endian
  header = htons(header);
  memcpy(message, &header, 2); // On copie 2 octets

  // On crée le numéro du message modulo 2^16
  uint16_t num = htons(params.num);
  memcpy(message + 2, &num, 2);

  // On crée les 2 octets représentant la hauteur/largueur de la grille
  uint16_t dimension = htons(params.hauteur << 8 | params.largeur);
  memcpy(message + 4, &dimension, 2);

  // On copie la grille (où chaque case est déjà au format Big Endian).
  memcpy(message + 6, params.grille, params.hauteur * params.largeur);

  return message;
}

/**
 * Créer un message pour transmettre une grille temporaire.
 * @param params Les paramètres du message.
 * @param game_type Le type de partie (vaut toujours 12).
 * @param player_id L'identifiant du joueur (vaut toujours 0).
 * @param team_id Le numéro de l'équipe du joueur (vaut toujours 0).
 * @param num Le numéro du message modulo 2^16.
 * @param nb_cases Le nombre de cases transmises.
 * @param grille Les cases de la grille temporaire.
 * @return Le message créé.
 */
uint8_t *ms_grid_tmp(msg_grid_tmp_t params) {
  /*
   * - CODEREQ vaut 12.
   * - ID et EQ valent 0.
   * - NUM est le numéro du message modulo 2^16. Il numérote les msg
   * multidiffusés toutes les freq ms.
   * - NB est le nombre de cases transmises.
   * - Chaque case est codée sur 3 octets:
   * -> 1er octet: numéro de ligne.
   * -> 2e octet: numéro de colonne.
   * -> 3e octet: contenu de la case.
   */

  uint8_t *message = malloc(sizeof(uint8_t) * 5 + 3 * params.nb_cases);

  // On crée les deux premiers octets du message (appelés 'header')
  uint16_t header =
      (params.game_type << 3) | (params.player_id << 1) | params.team_id;
  // Les deux premiers octets sont au format Big Endian
  header = htons(header);
  memcpy(message, &header, 2); // On copie 2 octets

  // On crée le numéro du message modulo 2^16
  uint16_t num = htons(params.num);
  memcpy(message + 2, &num, 2);

  // On crée l'octet représentant le nombre de cases
  uint8_t nb_cases = htons(params.nb_cases);
  memcpy(message + 4, &nb_cases, 1);

  // On copie les cases (où chaque case est déjà au format Big Endian).
  memcpy(message + 5, params.grille, 3 * params.nb_cases);

  return message;
}

/**
 * Créer un message pour indiquer la fin de la partie.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint16_t ms_end_game(msg_end_game_t params) {
  /* CODEREQ vaut 15 si la partie est en mode 4 joueurs, 16 pour le mode équipe.
   * ID est l'identifiant du joueur gagant si CODEREQ vaut 15, ignoré sinon.
   * EQ est le numéro de l'équipe gagante si CODEREQ vaut 16, ignoré sinon. */
  uint16_t message =
      (params.game_type << 3) | (params.player_id << 1) | (params.team_id);
  return message;
}

/**
 * Créer un message pour transmettre un message au tchat.
 * @param params Les paramètres du message.
 * @return Le message créé.
 */
uint8_t *ms_tchat(msg_tchat_t params) {
  // NOTE: Quel type est renvoyé, étant donnée que le message est de longueur
  // variable ?
  /* CODEREQ vaut 7 si le message est destiné à un joueur, 8 pour une équipe.
   * ID est l'id du joueur qui envoie le message.
   * EQ est l'id de l'équipe qui envoie le message, si cela a du sens.
   * LEN est un entier représentant le nb de caractères du texte à transmettre.
   * DATA est le texte à transmettre. */

  uint8_t *msg = malloc(3 + params.len);

  *((uint16_t *)msg) =
      htons(params.dest << 3 | params.player_id << 1 | params.team_id);
  msg[2] = (uint8_t)params.len;
  memcpy(&(msg[3]), params.data, params.len);

  return msg;
}

/* ************************ Fonctions de réception ************************ */

/**
 * Extraire le type de partie d'un message 'join'.
 * @param message Le message.
 * @return Le type de partie.
 */
msg_join_ready_t mg_join(uint16_t message) {
  // NOTE: Est-ce qu'on en renverrait pas plutôt juste le type de partie ?
  msg_join_ready_t params;

  // On récupère le message au format Little Endian
  message = ntohs(message);

  // On masque tous les bits, sauf les trois de poids faible
  params.game_type = message >> 3;
  // Les champs ID et EQ sont ignorés.
  params.player_id = 0;
  params.team_id = 0;
  return params;
}

/**
 * Extraire toutes les informations d'un message 'ready'.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_join_ready_t mg_ready(uint16_t message) {
  msg_join_ready_t params;

  // On récupère le message au format Little Endian
  message = ntohs(message);

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
msg_game_t mg_game(uint32_t message) {
  msg_game_t params;

  // On récupère le header du message
  uint16_t header = message >> 16;
  // On convertit en Little Endian
  header = ntohs(header);

  // On récupère le footer du message
  uint16_t footer = message & 0xFFFF;
  // On convertit en Little Endian
  footer = ntohs(footer);

  params.game_type = header >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (header >> 1) & 0x3;
  // On masque tous les bits, sauf celui de poids faible
  params.team_id = header & 0x1;
  params.num = footer >> 3;
  // On masque tous les bits, sauf les trois de poids faible
  params.action = footer & 0x7;

  return params;
}

/**
 * Extraire toutes les informations d'un message d'intégration.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_game_data_t mg_game_data(uint8_t *message) {
  msg_game_data_t params;

  uint16_t header;
  // On copie les 2 premiers octets du message
  memcpy(&header, message, 2);
  // On convertit en Little Endian
  header = ntohs(header);

  params.game_type = header >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (header >> 1) & 3;
  // On masque tous les bits, sauf celui de poids faible
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

/**
 * Extraire toutes les informations d'un message de grille de jeu.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_grid_t mg_game_grid(uint8_t *message) {
  msg_grid_t params = {0};

  uint16_t header;
  // On copie les 2 premiers octets du message
  memcpy(&header, message, 2);
  // On convertit en Little Endian
  header = ntohs(header);

  params.game_type = header >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (header >> 1) & 3;
  // On masque tous les bits, sauf celui de poids faible
  params.team_id = header & 1;

  // On récupère le numéro du message
  uint16_t num;
  memcpy(&num, message + 2, 2);
  params.num = ntohs(num);

  // On récupère la hauteur et la largeur de la grille
  uint16_t dimension;
  memcpy(&dimension, message + 4, 2);
  dimension = ntohs(dimension);
  params.hauteur = dimension >> 8;
  params.largeur = dimension & 0xFF;

  // Affichage de la hauteur et de la largeur
  printf("mg: Hauteur: %d\n", params.hauteur);
  printf("mg: Largeur: %d\n", params.largeur);

  // On copie la grille
  memcpy(params.grille, message + 6, params.hauteur * params.largeur);

  return params;
}

/**
 * Extraire toutes les informations d'un message de grille temporaire.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_grid_tmp_t mg_grid_tmp(uint8_t *message) {
  msg_grid_tmp_t params;

  uint16_t header;
  // On copie les 2 premiers octets du message
  memcpy(&header, message, 2);
  // On convertit en Little Endian
  header = ntohs(header);

  params.game_type = header >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (header >> 1) & 3;
  // On masque tous les bits, sauf celui de poids faible
  params.team_id = header & 1;

  // On récupère le numéro du message
  uint16_t num;
  memcpy(&num, message + 2, 2);
  params.num = ntohs(num);

  // On récupère le nombre de cases
  uint8_t nb_cases;
  memcpy(&nb_cases, message + 4, 1);
  params.nb_cases = ntohs(nb_cases);

  // On copie les cases
  memcpy(params.grille, message + 5, 3 * params.nb_cases);

  return params;
}

/**
 * Extraire toutes les informations d'un message de fin de partie.
 * @param message Le message.
 * @return Les informations extraites.
 */
msg_end_game_t mg_end_game(uint16_t message) {
  msg_end_game_t params;

  params.game_type = message >> 3;
  // On masque tous les bits, sauf les deux de poids faible
  params.player_id = (message >> 1) & 0x3;
  // On masque tous les bits, sauf celui de poids faible
  params.team_id = message & 0x1;

  return params;
}

msg_tchat_t mg_tchat(uint8_t *message) {
  msg_tchat_t acc;
  uint16_t head = ntohs(*((uint16_t *)message));

  acc.player_id = head & 0b110;
  acc.team_id = head & 0b1;
  acc.len = message[3];
  acc.data = malloc(acc.len + 1);
  memcpy(acc.data, &(message[4]), acc.len);
  acc.data[acc.len] = 0;

  return acc;
}
