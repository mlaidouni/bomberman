#include "partie.h"

#include "../lib/constants.h"
#include <sys/time.h>
#include <unistd.h>
/* ********** Fonctions de partie ********** */

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

  /* ********** Configuration de la surveillance des descripteurs ********** */

  // On initialise les stock de messages
  mp_t mp = {0};
  init_mp(&mp, partie);

  /* ********** Configuration des compteurs de temps ********** */

  struct timeval last_clock, start_clock, end_clock;
  last_clock.tv_sec = 0;
  gettimeofday(&start_clock, NULL);

  // On envoie la grille initiale à tous les joueurs
  if (send_game_grid(partie, board) < 0)
    return -1;

  // Tant que la partie n'est pas terminée
  while (partie->end) {

    /* On surveille l'ensemble des sockets mp.socks, avec 1 socket (la socket
     * mdiff de la partie), avec un timeout. La fréquence de l'examen des
     * requêtes est de FREQ ms.
     * On bloque ici jusqu'à ce qu'une socket mdiff soit prête. */
    int r = poll(mp.socks, 1, FREQ);
    // Gestion des erreurs
    if (r < 0) {
      perror("partie.c: start_game(): poll()");
      return -1;
    }

    /* ******** Reception action d'un joueur et actualisation grille ******** */

    // Si on a recu un message sur la socket de multidiffusion
    if (partie->sock_mdiff & POLLIN) {

      puts("\033[32mpartie.c: start_game(): Message reçu en "
           "multidiffusion\033[0m");

      // On reçoit le message de jeu
      msg_game_t mg;
      r = recv_msg_game(&mg, mp.partie->sock_mdiff);
      // Gestion des erreurs
      if (r < 0) {
        perror("partie.c: start_game(): recv_msg_game()");
        return -1;
      }

      // On affiche l'id du joueur et l'action effectuée
      printf("partie.c: start_game(): Joueur %d, action %d\n", mg.player_id,
             mg.action);

      // On met à jour mp
      update_mp(&mp, &mg);

      // On actualise la grille de jeu
      action_player(board, mg.player_id, mg.action);
    }

    /* ********** Envoie de la grille complète ********** */

    // Si une seconde s'est écoulée, on envoie la grille en multidiffusion
    if ((start_clock.tv_sec - last_clock.tv_sec) >= 1) {
      last_clock = start_clock;

      if (send_game_grid(partie, board) < 0)
        return -1;

    } else {

      /*
       * Diffusion de la grille de jeu temporaire
       */
    }

    /* ********** Gestion de la fin de la partie ********** */

    // TODO: Penser à gérer la fermeture des sockets des clients
    // ...

    // On usleep le temps d'atteindre FREQ * 1000 microsecondes on fait une
    // difference entre le temps actuel et le temps de debut de la boucle
    gettimeofday(&end_clock, NULL);
    usleep(FREQ * 1000 - (end_clock.tv_usec - start_clock.tv_usec));
    gettimeofday(&start_clock, NULL);
  }

  return 0;
}

/**
 * Initialise la structure mp_t de la partie.
 * @param mp La structure à initialiser.
 * @param sock_mdiff La socket de multidiffusion.
 */
void init_mp(mp_t *mp, partie_t *partie) {
  // On initialise la structure
  memset(mp, 0, sizeof(mp_t));

  // On initialise la partie
  mp->partie = partie;

  // On initialise les sockets à surveiller
  memset(mp->socks, 0, sizeof(mp->socks));
  struct pollfd socks[1];
  memset(socks, 0, sizeof(socks));

  // La socket mdiff de la partie sera toujours identifiée par l'indice 0
  mp->socks[0].fd = mp->partie->sock_mdiff;
  // On la surveille en lecture
  mp->socks[0].events = POLLIN;

  // On initialise les listes de messages
  for (int i = 0; i < 4; i++) {
    mp->move[i] = init_list();
    mp->bomb[i] = init_list();
  }
}

/**
 * Met à jour la structure mp_t de la partie.
 * @param mp La structure à mettre à jour.
 * @param mg Le message de jeu reçu.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int update_mp(mp_t *mp, msg_game_t *mg) {
  puts("\033[31m-> partie.c: update_mp(): Mise à jour des messages de "
       "partie...\033[0m");

  // On récupère l'id du joueur
  int player_id = mg->player_id;
  printf("partie.c: update_mp(): Joueur %d\n", player_id);

  // On récupère le type d'action effectuée
  int action = mg->action;
  printf("partie.c: update_mp(): Action %d\n", action);

  // On récupère la liste de mouvements et de bombes du joueur
  list *move = mp->move[player_id];
  // list *bomb = mp->bomb[player_id];

  // Si l'action est une annulation, on modifie les listes bomb ou move
  if (action == 5) {
    puts("\033[32m-> partie.c: update_mp(): Annulation...\033[0m");
    // ...
  }
  // Si l'action effectuée n'est pas un déplacement, on modifie la list bomb
  else if (action == 4) {
    puts("\033[32m-> partie.c: update_mp(): Bombe...\033[0m");
    // ...
  }
  // Sinon, on travaille sur la liste move
  else if (0 <= action && action <= 3) {
    puts("\033[32m-> partie.c: update_mp(): Déplacement...\033[0m");

    // On récupère la liste de mouvements du joueur
    // list *move = mp->move[player_id];

    msg_game_t *msg = malloc(sizeof(msg_game_t));
    *msg = *mg;

    // On ajoute le message à la liste des messages de mouvements
    add_head(move, msg);

    // Pour chaque message de la liste, on affiche son numéro et l'action
    // effectuée
    for (list_elem *n = move->out; n != NULL; n = n->next) {
      msg_game_t *m = n->curr;
      printf("partie.c: update_mp(): Message %d, action %d\n", m->player_id,
             m->action);
    }

  } else {
    printf("\033[31m partie.c: update_mp(): Action invalide\033[0m\n");
    return -1;
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

/**
 * Recoit un message de jeu.
 * @param mg La structure msg_game_t à remplir.
 * @param sock_mdiff La socket de multidiffusion.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int recv_msg_game(msg_game_t *mg, int sock_mdiff) {
  puts("\033[31m-> partie.c: recv_msg_game(): Réception d'un message de "
       "jeu...\033[0m");
  uint32_t *msg = malloc(sizeof(uint32_t));
  int r = recvfrom(sock_mdiff, msg, sizeof(msg), 0, NULL, NULL);

  printf("partie.c: recv_msg_game(): Message reçu : %d\n", *msg);

  // Gestion des erreurs
  if (r < 0) {
    perror("partie.c: start_game(): recvfrom()");
    return -1;
  }
  if (r == 0) {
    fprintf(stderr, "partie.c: start_game(): recvfrom(): socket fermée\n");
    return -1;
  }

  // On convertit le message en structure msg_game_t
  *mg = mg_game(*msg);

  // On libère la mémoire
  free(msg);

  return 0;
}

/**
 * Envoie la grille de jeu en multidiffusion.
 * @param partie La partie en cours.
 * @param board La grille de jeu.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int send_game_grid(partie_t *partie, board board) {
  // On initialise la structure msg_grid_t avec la grille de jeu
  msg_grid_t grid = init_msg_grid(partie, board);

  // On convertit la structure en message
  uint8_t *msg = ms_game_grid(grid);

  // Envoi du message en multidiffusion à tous les joueurs
  // FIX: magical number
  size_t msgsz = grid.hauteur * grid.largeur * sizeof(uint8_t) + 6;

  if (sendto(partie->sock_mdiff, msg, msgsz, 0,
             (struct sockaddr *)&partie->g_adr, sizeof(partie->g_adr)) < 0) {
    perror("partie.c: start_game(): sendto()");
    close(partie->sock_mdiff);
    // On libère la mémoire
    free(grid.grille);
    free(msg);
    return -1;
  } else
    printf("partie.c: start_game(): Message envoyé en multidiffusion\n");

  return 0;
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
 * @return L'id de la partie créée (ie un nombre >= 0). un nombre negatif si
 * il y a une erreur
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
  // FIX: Check la limite qu'on peut atteindre, i.e :2:10000: est-elle valide
  // ?
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
