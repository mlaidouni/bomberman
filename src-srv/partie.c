#include "partie.h"

/**
 * Lance le jeu.
 * @param partie La partie à lancer.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int start_game(partie_t partie) {
  // TODO: Gérer le type de partie et les parties en attente

  while (partie.nb_joueurs < 4)
    // Si le nombre de joueurs est inférieur à 4, on attend des connexions
    ;

  // Si le nombre de joueurs est égal à 4, on lance et gère le jeu
  // Tant que la partie n'est pas terminée
  while (partie.end) {

    // TODO: Le jeu
    // ...

    // TODO: Penser à gérer la fermeture des sockets des clients
    // ...
  }

  return 0;
}

/**
 * Crée une partie.
 * @param client Le client qui a demandé la partie.
 * @param params Les paramètres de la partie.
 * @return La partie créée.
 */
partie_t create_partie(client_t client, msg_join_ready_t params) {
  // Création de la structure partie
  partie_t partie = {0};
  partie.end = 1;
  partie.type = params.game_type;
  partie.nb_joueurs = 1;

  // Création de l'adresse multicast
  memset(&partie.g_adr, 0, sizeof(partie.g_adr));
  partie.g_adr.sin6_family = AF_INET6;
  // On initialise l'adresse ipv6 de l'adresse de groupe
  generate_multicast_adr(partie.adr_mdiff, sizeof(partie.adr_mdiff));
  inet_pton(AF_INET6, partie.adr_mdiff, &partie.g_adr);
  // On initialise l'interface locale de multicast
  partie.g_adr.sin6_scope_id = 0; // 0 pour interface par défaut

  // Création de l'adresse de réception
  memset(&partie.r_adr, 0, sizeof(partie.r_adr));
  partie.r_adr.sin6_family = AF_INET6;
  // On recoit des messages de n'importe quelle adresse locale
  partie.r_adr.sin6_addr = in6addr_any;

  // On initialise les ports
  generate_multicast_ports(&partie.port_mdiff, &partie.port_udp);
  partie.g_adr.sin6_port = htons(partie.port_mdiff);
  partie.r_adr.sin6_port = htons(partie.port_udp);

  // Création d'un joueur
  joueur_t j = {0};
  j.client = client;
  j.id = partie.nb_joueurs;

  // On ajoute le joueur à la partie
  partie.joueurs[partie.nb_joueurs] = j;

  // On incrémente le nombre de joueurs
  partie.nb_joueurs++;

  // Si la partie est en mode 2 équipes, on répartit les joueurs
  if (partie.type == 1)
    /* NOTE: On choisit de les répartir en fonction de leur ordre
     * d'arrivée (i.e, de ordre de connexion) */
    j.team = partie.nb_joueurs % 2;

  // On ajoute la partie à la liste des parties
  srv.parties.parties[srv.parties.nb_parties] = partie;
  srv.parties.nb_parties++;
  return partie;
}

/**
 * Ajoute un joueur à une partie.
 * @param partie La partie à laquelle ajouter le joueur.
 * @param client Le client à ajouter.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int add_joueur(partie_t partie, client_t client) {
  // Création d'un joueur
  joueur_t j = {0};
  j.client = client;
  j.id = partie.nb_joueurs;

  // On ajoute le joueur à la partie
  partie.joueurs[partie.nb_joueurs] = j;

  // On incrémente le nombre de joueurs
  partie.nb_joueurs++;

  // Si la partie est en mode 2 équipes, on répartit les joueurs
  if (partie.type == 1)
    /* NOTE: On choisit de les répartir en fonction de leur ordre
     * d'arrivée (i.e, de ordre de connexion) */
    j.team = partie.nb_joueurs % 2;

  return 0;
}

/**
 * Trouve une partie en fonction de son type.
 * @param type Le type de la partie à trouver.
 * @return L'indice de la partie trouvée dans la liste des parties du serveur,
 * -1 sinon.
 */
int find_partie(int type) {
  // On cherche la partie correspondant au type
  for (int i = 0; i < srv.parties.nb_parties; i++)
    if (srv.parties.parties[i].type == type)
      return i;

  return -1;
}

/**
 * Génère une adresse multicast pour une nouvelle partie.
 * @param adr L'adresse multicast générée.
 * @param size La taille du tableau contenant l'adresse.
 */
void generate_multicast_adr(char *adr, size_t size) {
  // On initialise l'adresse
  memset(adr, 0, size);

  // On génère une adresse multicast
  // FIXME: Check la limite qu'on peut atteindre, i.e :2:10000: est-il valide ?
  sprintf(adr, "ff12:1:2:%d", srv.parties.nb_parties);
}

/**
 * Génère les ports multicast et udp pour une nouvelle partie.
 * @param port_mdiff Le port multicast généré.
 * @param port_udp Le port udp généré.
 */
void generate_multicast_ports(int *port_mdiff, int *port_udp) {
  // On génère les ports
  *port_mdiff = 5000 + srv.parties.nb_parties;
  *port_udp = 7000 + srv.parties.nb_parties;
}
