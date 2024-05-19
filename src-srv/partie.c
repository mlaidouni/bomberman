#include "partie.h"

/**
 * Lance et gère une partie.
 * @param partie La partie à lancer.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int start_game(partie_t *partie) {
  puts("\033[32mpartie.c: start_game: La partie vient d'être lancée !\033[0m");

  // On initialise la grille de jeu
  board board = {0};
  init_board(&board, partie->type);

  // Tant que la partie n'est pas terminée
  while (partie->end) {

    /* ********** Envoie de la grille complète ********** */

    // On initialise la structure msg_grid_t avec la grille de jeu
    msg_grid_t grid = init_msg_grid(partie, board);

    // On convertit la structure en message
    uint8_t *msg = ms_game_grid(grid);

    // Envoi du message en multidiffusion à tous les joueurs
    // FIX: magical number
    size_t msgsz = grid.hauteur * grid.largeur * sizeof(uint8_t) + 6;

    // Affichage du contenu de chaque case
    puts("\033[31m while... Affichage de la grille... \033[0m");
    affichetmpgrid(grid);

    if (sendto(partie->sock_mdiff, msg, msgsz, 0,
               (struct sockaddr *)&partie->g_adr, sizeof(partie->g_adr)) < 0) {
      perror("partie.c: start_game(): sendto()");
      close(partie->sock_mdiff);
      // On libère la mémoire
      free(grid.grille);
      free(msg);
      return -1;
    }

    // On libère
    free(grid.grille);

    break; // TODELETE: (debug) On arrête la boucle après un envoi

    /* ********** Reception des messages de joueurs ********** */

    // TODO: Le jeu
    // ...

    /* ********** Gestion de la fin de la partie ********** */

    // TODO: Penser à gérer la fermeture des sockets des clients
    // ...
  }

  return 0;
}

/* ********** Fonctions de messages de partie ********** */

/**
 * Initialise la structure msg_grid_t avec la grille de jeu.
 * @param partie La partie en cours.
 * @param board La grille de jeu.
 * @return La structure msg_grid_t initialisée.
 */
msg_grid_t init_msg_grid(partie_t *partie, board board) {
  msg_grid_t grid;
  memset(&grid, 0, sizeof(msg_grid_t));
  // On récupère les dimensions de la grille depuis la structure board
  grid.hauteur = HEIGHT;
  grid.largeur = WIDTH;

  // On initialise la grille
  int len_grille = grid.hauteur * grid.largeur * sizeof(uint8_t);
  grid.grille = malloc(len_grille); // FIXME: TO FREE
  // On copie les données de la board dans la grille
  memcpy(grid.grille, board.grid, len_grille);

  // On dispose les joueurs sur la grille
  for (int i = 0; i < partie->nb_joueurs; i++) {
    // On récupère la position du joueur
    pos p = board.players[i].pos;
    // On récupère l'identifiant du joueur
    int id = partie->joueurs[i].id;

    // On met à jour la grille avec la position du joueur
    grid.grille[p.x + p.y * WIDTH] = id + 5;
  }

  return grid;
}

/* ********** Fonctions de création & gestion de partie ********** */

/**
 * Crée une nouvelle partie avec le client donné, ou ajoute celui-ci à une
 * partie déjà existante.
 * @param client Le client qui a créé la partie.
 * @param params Les paramètres de la partie.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int init_partie(msg_join_ready_t params, client_t client) {
  /* On vérifie la validité du message, e.g que le type de partie est
  valide */
  if (params.game_type != 0 && params.game_type != 1) {
    fprintf(stderr, "partie.c: init_partie(): type de partie invalide\n");
    return -1;
  }

  // On récupère la liste des parties de ce type
  int finded_partie = find_partie(params.game_type);

  // On cherche une partie du type demandé qui n'est pas pleine
  // Si on n'en trouve pas, on crée une nouvelle partie
  if (finded_partie == -1) {
    // On crée et ajoute la partie à la liste des parties
    if (create_partie(client, params) < 0)
      return -1;
  } else
    // Sinon, on ajoute le joueur à la partie trouvée
    add_joueur(&srv.parties.parties[finded_partie], client);

  return 0;
}

/**
 * Crée une partie et l'ajoute à la liste des parties du serveur.
 * @param client Le client qui a demandé la partie.
 * @param params Les paramètres de la partie.
 * @return L'id de la partie créée (ie un nombre >= 0). un nombre negatif si il
 * y a une erreur
 */
int create_partie(client_t client, msg_join_ready_t params) {
  // Création de la structure partie
  partie_t partie = {.nb_joueurs = 0, .end = 1, .type = params.game_type};

  // Création de la socket multicast
  int sock = socket(PF_INET6, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("partie.c: socket(): création de la socket échouée");
    return -1;
  }
  partie.sock_mdiff = sock;

  // Création de l'adresse multicast
  memset(&partie.g_adr, 0, sizeof(partie.g_adr));
  partie.g_adr.sin6_family = AF_INET6;
  // On initialise l'adresse ipv6 de l'adresse de groupe
  generate_multicast_adr(partie.adr_mdiff, sizeof(partie.adr_mdiff));
  // Afficher l'adresse de multidiffusion
  printf("Adresse de multidiffusion : %s\n", partie.adr_mdiff);
  inet_pton(AF_INET6, partie.adr_mdiff, &partie.g_adr.sin6_addr);
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

  // Si aucun joueur n'est en attente, on crée un tableau de joueurs
  if (partie.nb_joueurs == 0)
    partie.joueurs = malloc(sizeof(joueur_t));

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
int add_joueur(partie_t *partie, client_t client) {
  // Création d'un joueur
  joueur_t j = {0};
  j.client = client;
  j.id = partie->nb_joueurs;

  // On réalloue la mémoire pour le tableau de joueurs
  partie->joueurs =
      realloc(partie->joueurs, (partie->nb_joueurs + 1) * sizeof(joueur_t));

  // On ajoute le joueur à la partie
  partie->joueurs[partie->nb_joueurs] = j;

  // On incrémente le nombre de joueurs
  partie->nb_joueurs++;

  // Si la partie est en mode 2 équipes, on répartit les joueurs
  if (partie->type == 1)
    /* NOTE: On choisit de les répartir en fonction de leur ordre
     * d'arrivée (i.e, de ordre de connexion) */
    j.team = partie->nb_joueurs % 2;

  return 0;
}

/**
 * Trouve une partie disponible en fonction de son type.
 * @param type Le type de la partie à trouver.
 * @return L'indice de la partie trouvée dans la liste des parties du serveur,
 * -1 sinon.
 */
int find_partie(int type) {
  // On cherche la partie correspondant au type et qui n'est pas pleine
  for (int i = 0; i < srv.parties.nb_parties; i++) {
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
  // FIX: Check la limite qu'on peut atteindre, i.e :2:10000: est-elle valide ?
  sprintf(adr, "ff12::1:2:%d", srv.parties.nb_parties);
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

/**
 * Libère la mémoire allouée pour une partie.
 * @param partie La partie à libérer.
 */
void free_partie(partie_t *partie) {
  // On libère la mémoire des joueurs
  free(partie->joueurs);
  // On ferme la socket
  close(partie->sock_mdiff);

  // On libère la mémoire de la partie
  free(partie);
}
