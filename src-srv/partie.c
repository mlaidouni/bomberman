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
 * Crée une nouvelle partie avec le client donné, ou ajoute celui-ci à une
 * partie déjà existente.
 * @param client Le client qui a créé la partie.
 * @param params Les paramètres de la partie.
 * @return La partie créée, ou la partie trouvée si elle existe déjà.
 */
partie_t init_partie(int type, client_t client) {
  partie_t partie = {.nb_joueurs = 0, .end = 1, .type = type};
  // On récupère la liste des parties de ce type
  /*int finded_partie = find_partie(params.game_type);



  // On cherche une partie du type demandé qui n'est pas pleine
  // Si on n'en trouve pas, on crée une nouvelle partie
  if (finded_partie == -1 || srv.parties.parties[finded_partie].nb_joueurs == 4)
    partie = create_partie(client, params);
  else {
    // Sinon, on ajoute le joueur à la partie trouvée
    partie = srv.parties.parties[finded_partie];
    add_joueur(partie, client);
  }

  return partie;*/
  return partie;
}

/**
 * Crée une partie.
 * @param client Le client qui a demandé la partie.
 * @param params Les paramètres de la partie.
 * @return L'id de la partie créée (ie un nombre >= 0). un nombre negatif si il y a une erreur
 */
int create_partie(client_t client, msg_join_ready_t params) {
  // Création de la structure partie
  partie_t partie = {.nb_joueurs = 0, .end = 1, .type = params.game_type};
  partie.end = 1;
  // FIXME: gérer le fait que le type demandé soit invalide
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

  // Si aucune partie n'est en cours, on alloue la mémoire
  if (srv.parties.nb_parties == 0)
    srv.parties.parties = malloc(sizeof(partie_t));
  else {
    // Si la liste est déjà allouée, on réalloue la mémoire
    partie_t *tmp = realloc(srv.parties.parties,
                            (srv.parties.nb_parties + 1) * sizeof(partie_t));

    // FIXME: faire une vraie gestion des erreurs
    if (tmp == NULL) {
      perror("partie.c: create_partie(): realloc()");
      return -1;
    }
    // On met à jour la liste des parties
    srv.parties.parties = tmp;
  }

  // On ajoute la partie à la liste des parties
  srv.parties.parties[srv.parties.nb_parties] = partie;
  srv.parties.nb_parties++;
  return srv.parties.nb_parties;
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
  for (int i = 0; i < srv.parties.nb_parties; i++) {
    printf("srv.parties.partie.type: %d\n", srv.parties.parties[i].nb_joueurs);
    if (srv.parties.parties[i].type == type &&
        srv.parties.parties[i].nb_joueurs < 4)
      return i;
  }
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
