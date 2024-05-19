#include "client.h"

void affiche_data_partie(msg_game_data_t *game_data, char *adr_mdiff) {
  printf(
      "\033[35m Données de la partie reçues:\n\t Adresse multicast: %s \n\t "
      "adr_mdiff: %d \n\t port_mdiff: %d \n\t "
      "port_udp:%d \n\t game_type: %d \n\t player_id: %d \n\t team_id: "
      "%d\n\033[0m",
      adr_mdiff, *(game_data->adr_mdiff), game_data->port_mdiff,
      game_data->port_udp, game_data->game_type, game_data->player_id,
      game_data->team_id);
}

int get_grille(msg_grid_t grid, int x, int y) {
  return (int)grid.grille[y * grid.largeur + x];
}

void affiche(msg_grid_t grid) {
  int x, y;
  for (y = 0; y < grid.hauteur; y++) {
    for (x = 0; x < grid.largeur; x++) {
      char c;
      switch (get_grille(grid, x, y)) {
      case 0:
        c = '?';
        break;
      case 1:
        c = '|';
        break;
      case 2:
        c = '-';
        break;
      case 3:
        c = '*';
        break;
      case 4:
        c = 'X';
        break;
      }
      mvaddch(y + 1, x + 1, c);
    }
  }
  refresh();
}

int main(int argc, char const *argv[]) {
  // NOTE: Le port devra être passé en argument
  const int port = 8081;

  /* ********** Connexion en mode TCP ********** */

  // Connexion en mode TCP au serveur
  int sock_client = connect_to_server(port);
  // Gestion des erreurs
  if (sock_client == -1)
    exit(EXIT_FAILURE);

  // printf("Connected to server !\n"); // TODELETE: debug

  /* ********** Gestion du choix de partie ********** */

  // On demande au joueur le type de partie qu'il veut rejoindre
  int game_type;
  printf("Entrer 0 pour jouer à 4 joueurs, 1 pour jouer en équipes: ");
  scanf("%d", &game_type);
  if (join_game(sock_client, game_type))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  /* ********** Gestion de la configuration de la partie ********** */

  // On attend la réception des données de la partie
  msg_game_data_t game_data;
  if (recv_msg_game_data(&game_data, sock_client))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  // On convertit l'adresse de uin8_t* à char*
  char adr_mdiff[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, game_data.adr_mdiff, adr_mdiff, INET6_ADDRSTRLEN);

  // affiche_data_partie(&game_data, adr_mdiff); // TODELETE: debug

  // Abonnement à l'adresse de multicast
  multicast_client_t mc;
  if (abonnement_mdiff(&mc, adr_mdiff, game_data.port_mdiff))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  // On s'annonce prêt à jouer au serveur
  if (ready(sock_client, game_type, game_data.player_id, 0))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  /* ********** Gestion des messages de la partie... ********** */

  // On initialise ncurses
  init_ncurses();

  while (1) {
    // On reçoit la grid de jeu
    msg_grid_t grid;
    if (recv_msg_game_grid(&grid, mc))
      exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

    affiche(grid);
    // Clear la fenêtre
    // clear();

    // Attend que l'utilisateur appuie sur une touche
    getch();

    // Ferme la fenêtre
    endwin();

    // TODO: Recv/send msg avec le serveur
  }
  // Fermeture de la socket TCP du client: à la fin de la partie seulement
  close(sock_client);
  return 0;
}

/* ********** Fonctions client ********** */

/**
 * Crée une socket client et se connecte à un serveur.
 * @param port Le port du serveur.
 * @return Le descripteur de la socket client ou -1 en cas d'erreur.
 */
int connect_to_server(int port) {
  // Création de la socket client
  int sock_client = socket(PF_INET6, SOCK_STREAM, 0);
  // Gestion des erreurs
  if (sock_client == -1) {
    perror("src-cli/client.c: connect_to_server: socket()");
    return -1;
  }

  // Préparation de l'adresse du destinataire (le serveur)
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;

  // Demande de connexion au serveur
  int r = connect(sock_client, (struct sockaddr *)&addr, sizeof(addr));
  // Gestion des erreurs
  if (r) {
    perror("src-cli/client.c: connect_to_server: connect()");
    close(sock_client);
    return -1;
  }
  return sock_client;
}

/**
 * Abonne un client à un groupe multicast.*
 * @param mc Structure contenant les informations de connexion du client.
 * @param adr_mdiff L'adresse du groupe multicast.
 * @param port_mdiff Le port du groupe multicast.
 * @return 0 si l'abonnement a réussi, -1 sinon.
 */
int abonnement_mdiff(multicast_client_t *mc, char *adr_mdiff, int port_mdiff) {
  // Variables d'erreur
  int r;

  // Initialisation de l'adresse
  memset(&mc->adr, 0, sizeof(mc->adr));

  // Déclarer la socket
  mc->sock = socket(PF_INET6, SOCK_DGRAM, 0);
  if (mc->sock == -1) {
    perror("client.c: abonnement_mdiff(): socket(): création socket failed");
    return -1;
  }

  // Autoriser la réutilisation de l'adresse locale
  int o = 1;
  r = setsockopt(mc->sock, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o));
  if (r < 0) {
    perror("client.c: abonnement_mdiff(): setsockopt(): REUSEADDR échoué");
    return -1;
  }

  // Initialisation de l'adresse de réception
  memset(&mc->adr, 0, sizeof(mc->adr));
  mc->adr.sin6_family = AF_INET6;
  mc->adr.sin6_addr = in6addr_any;
  mc->adr.sin6_port = htons(port_mdiff);

  // On lie la socket à l'adresse de réception (port multicast)
  r = bind(mc->sock, (struct sockaddr *)&mc->adr, sizeof(struct sockaddr_in6));
  if (r < 0) {
    perror("client.c: abonnement_mdiff(): bind(): bind socket au port failed");
    close(mc->sock);
    return -1;
  }

  // Abonnement au groupe multicast
  // Initialisation de la structure pour la multidiffusion
  inet_pton(AF_INET6, adr_mdiff, &mc->grp.ipv6mr_multiaddr.s6_addr);
  // Utilisation de l'interface multicast par défaut
  mc->grp.ipv6mr_interface = 0;

  // On s'abonne au groupe
  socklen_t len = sizeof(mc->grp);
  r = setsockopt(mc->sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mc->grp, len);
  if (r < 0) {
    perror("client.c: abonnement_mdiff(): setsockopt: abonnement échoué");
    close(mc->sock);
    return -1;
  }

  return 0;
}

// FIX: OBSOLETE: Envoie un message de type 'game' au serveur.
int action(int sock_client, int game_type, int player_id, int team_id, int num,
           int action) {

  // FIX: OBSOLETE -> Utilise send_message, qui est en TCP
  // Création de la structure du message
  msg_game_t params;
  params.game_type = game_type;
  params.player_id = player_id;
  params.team_id = team_id;
  params.num = num;
  params.action = action;

  // Création du message
  uint32_t message = ms_game(params);

  // Envoi du message
  int bytes = send_message(sock_client, &message, sizeof(message), "action");
  // Gestion des erreurs (en partie gérée par send_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets envoyés (voir s'ils ont tous été envoyés)

  return 0;
}

/* ********** Fonctions client ncurses ********** */

// Initialise ncurses
void init_ncurses() {
  /* Start curses mode */
  initscr();
  /* Disable line buffering */
  raw();
  /* No need to flush when intr key is pressed */
  intrflush(stdscr, FALSE);
  // Required in order to get events from keyboard
  keypad(stdscr, TRUE);
  // Make getch non-blocking
  // nodelay(stdscr, TRUE);
  /* Don't echo() while we do getch (we will manually print characters when
   * relevant) */
  noecho();
  // Set the cursor to invisible
  curs_set(0);
  // Enable colors
  start_color();
  // Define a new color style (text is yellow, background is black)
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
}

/* ********** Fonctions d'envoi de messages ********** */

/**
 * Envoie un message de type 'join' au serveur.
 * @param sock_client La socket TCP du client.
 * @param game_type Le type de jeu.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int join_game(int sock_client, int game_type) {
  // Création de la structure du message et du message
  msg_join_ready_t params;
  params.game_type = game_type;
  uint16_t message = ms_join(params);

  // Envoi du message
  int bytes = send_message(sock_client, &message, sizeof(message), "join_game");
  // Gestion des erreurs (en partie gérée par send_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets envoyés (voir s'ils ont tous été envoyés)

  return 0;
}

/**
 * Envoie un message de type 'ready' au serveur.
 * @param sock_client La socket TCP du client.
 * @param game_type Le type de jeu.
 * @param player_id L'identifiant du joueur.
 * @param team_id Le numéro d'équipe du joueur.
 * @return 0 si tout s'est bien passé, -1 sinon.s
 */
int ready(int sock_client, int game_type, int player_id, int team_id) {
  // Création de la structure du message
  msg_join_ready_t params;
  params.game_type = game_type;
  params.player_id = player_id;
  params.team_id = team_id;

  // Création du message
  uint16_t message = ms_ready(params);

  // Envoi du message
  int bytes = send_message(sock_client, &message, sizeof(message), "ready");
  // Gestion des erreurs (en partie gérée par send_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets envoyés (voir s'ils ont tous été envoyés)

  return 0;
}

/* ********** Fonctions de réception de messages ********** */

/**
 * Réception du msg_game_data du serveur.
 * @param game_data La structure a remplir.
 * @param sock_client La socket sur laquelle on réceptionne.
 * @return 0 si tout se passe bien, -1 sinon.
 */
int recv_msg_game_data(msg_game_data_t *game_data, int sock_client) {
  // FIXME: magic number, trouver comment récupérer la taille du message
  int len = sizeof(uint8_t) * 22;
  uint8_t *msg = malloc(len);

  puts("\033[35m Réception des game_data...\033[0m"); // TODELETE: debug

  int r = recv(sock_client, msg, len, 0);
  // Gestion des erreurs
  if (r == -1) {
    perror("client.c: recv_msg_game_data: recv");
    return -1;
  }

  // On récupère les données de la partie
  *game_data = mg_game_data(msg);

  // On libère la mémoire
  free(msg);

  return 0;
}

/**
 * Réception de la grid de jeu.
 * @param mc La structure multicast.
 * @return Un pointeur vers le message reçu.
 */
int recv_msg_game_grid(msg_grid_t *grid, multicast_client_t mc) {
  // FIXME: magic number, trouver comment récupérer la hauteur et la largeur
  int len = sizeof(uint8_t) * (6 + HEIGHT * WIDTH);
  uint8_t *msg = malloc(len);
  socklen_t len_adr = sizeof(mc.adr);

  puts("\033[35m Réception de la grid...\033[0m"); // TODELETE: debug

  // Réception des données
  int r = recvfrom(mc.sock, msg, len, 0, (struct sockaddr *)&mc.adr, &len_adr);
  //  Gestions des erreurs
  if (!r || r < 0) {
    perror("client.c: recv_grid: recvfrom");
    return -1;
  }

  // On récupère les données de la grid
  *grid = mg_game_grid(msg);

  // On libère la mémoire
  free(msg);

  return 0;
}

/* ********** Fonctions utilitaires ********** */

/**
 * Envoie un message au serveur en TCP.
 * @param sock_client La socket client.
 * @param message Un pointeur vers le message à envoyer.
 * @param msg_size La taille du message.
 * @param type Le type d'envoie, i.e le nom de la fonction appelante.
 * @return Le nombre d'octets envoyés ou -1 en cas d'erreur.
 */
int send_message(int sock_client, void *message, size_t msg_size, char *type) {
  // Envoi du message
  int r = send(sock_client, message, msg_size, 0);

  // Gestion des erreurs
  if (r == -1) {
    char err_msg[100];
    sprintf(err_msg, "client.c: %s: send message failed", type);
    perror(err_msg);
    close(sock_client);
    return -1;
  }

  return r;
}
