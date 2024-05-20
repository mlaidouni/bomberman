

#include "client.h"
#include "../lib/constants.h"
#include <poll.h>

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

/**
 * Récupère la valeur d'une case de la grille.
 * @param grid La grille.
 * @param x La coordonnée x de la case.
 * @param y La coordonnée y de la case.
 * @return La valeur de la case.
 */
int get_grille(msg_grid_t grid, int x, int y) {

  return (int)grid.grille[y * grid.largeur + x];
}

/**
 * Affiche la grille de jeu avec ncurses.
 * @param grid La grille à afficher.
 */
void affiche(msg_grid_t grid) {
  int x, y;
  for (y = 0; y < grid.hauteur; y++) {
    for (x = 0; x < grid.largeur; x++) {
      // On récupère le caractère associé à la valeure de la case
      char c;
      switch (get_grille(grid, x, y)) {
      case 0:
        c = '_';
        break;
      case 1:
        c = '|';
        break;
      case 2:
        c = '#';
        break;
      case 3:
        c = '.';
        break;
      case 4:
        c = 'x';
        break;
      case 5:
        c = '0';
        break;
      case 6:
        c = '1';
        break;
      case 7:
        c = '2';
        break;
      case 8:
        c = '3';
        break;
      }

      // On affiche la case, avec un espace entre chaque case.
      mvaddch(y + 1, 2 * x + 1, c);
    }
  }
  refresh();
}

ACT action_command() {
  int c;
  int prev_c = ERR;
  // We consume all similar consecutive key presses
  while ((c = getch()) !=
         ERR) { // getch returns the first key press in the queue
    if (prev_c != ERR && prev_c != c) {
      ungetch(c); // put 'c' back in the queue
      break;
    }
    prev_c = c;
  }
  ACT a;
  switch (prev_c) {
  case ERR:
    break;
  case KEY_LEFT:
    a = A_LEFT;
    break;
  case KEY_RIGHT:
    a = A_RIGHT;
    break;
  case KEY_UP:
    a = A_UP;
    break;
  case KEY_DOWN:
    a = A_DOWN;
    break;
  case KEY_B2:
    a = A_BOMB;
    break;
  case 'q':
    a = A_QUIT;
    break;

  case 't':
    a = A_TCHAT;
    break;
  }
  return a;
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
  if (join_game(sock_client, game_type + 1))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  /* ********** Gestion de la configuration de la partie ********** */

  // On attend la réception des données de la partie
  msg_game_data_t game_data;
  if (recv_msg_game_data(&game_data, sock_client))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  player_client player = {game_data.player_id, game_data.team_id};
  // On convertit l'adresse de uin8_t* à char*
  char adr_mdiff[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, game_data.adr_mdiff, adr_mdiff, INET6_ADDRSTRLEN);

  // affiche_data_partie(&game_data, adr_mdiff); // TODELETE: debug

  /* Configuration de l'envoie et réception UDP (abonnement à l'adresse de
   * multicast + adresse d'envoie de messages) */
  multicast_client_t mc;
  if (config_udp(&mc, adr_mdiff, game_data))
    exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

  // On s'annonce prêt à jouer au serveur
  if (ready(sock_client, game_type + 3, game_data.player_id, 0))
    printf("game_type: %d, player_id: %d, team_id: %d\n", game_type,
           game_data.player_id, game_data.team_id);

  /* ********** Gestion des messages de la partie... ********** */

  struct pollfd fds[2];
  fds[0].fd = mc.sock;
  fds[0].events = POLLIN;
  fds[1].fd = sock_client;
  fds[1].events = POLLIN;

  // On reçoit la grid de jeu
  msg_grid_t grid;

  if (recv_msg_game_grid(&grid, mc))
    exit(EXIT_FAILURE);

  // On initialise ncurses
  init_ncurses();

  affiche(grid);

  int num = 0;
  while (1) {
    // On attend un message de la partie
    int r = poll(fds, 2, FREQ * 10);

    // Gestion des erreurs
    if (r == -1) {
      perror("client.c: main: poll()");
      exit(EXIT_FAILURE);
    }

    if (fds[0].revents & POLLIN) {

      if (recv_msg_game_grid(&grid, mc))
        exit(EXIT_FAILURE); // En cas d'échec on exit, pour l'instant.

      // On reçoit la grid de jeu
      // if (recv_msg_grid_tmp(&grid, mc))
      // exit(EXIT_FAILURE);
      init_ncurses();
      affiche(grid);
    }
    char buffer[100];

    ACT a = action_command();
    if (a == A_QUIT) {
      // Ferme la fenêtre
      break;
    }

    if (a == A_TCHAT) {
      endwin();

      msg_game_t params = {grid.game_type, player.player_id, player.team, num,
                           a};
      uint32_t message = ms_game(params);

      r = sendto(mc.sock, &message, sizeof(message), 0,
                 (struct sockaddr *)&mc.s_adr, sizeof(mc.s_adr));
      if (r == -1) {
        perror("client.c: main: sendto()");
        exit(EXIT_FAILURE);
      }

      if (fds[1].revents & POLLIN) {
        puts("Message reçu");

        memset(buffer, 0, 100);
        uint8_t *mess = malloc(sizeof(uint8_t) * 5 + 100);
        int r = recv(sock_client, mess, sizeof(uint8_t) * 5 + 100, 0);
        if (r == -1) {
          perror("client.c: main: recv()");
          exit(EXIT_FAILURE);
        }

        printf("Message reçu: %d\n", r);
        printf("Message reçu: %s\n", mess);

        msg_tchat_t msg = mg_tchat(mess);
        // print en jaune
        fprintf(stdin, "\033[33mjoueur %d: %s\033[0m\n", msg.player_id,
                msg.data);
        memset(buffer, 0, 100);
      }
      memset(buffer, 0, 100);

      printf("Enter a message: ");
      scanf("%s", buffer);
      if (buffer[0] != 'b') {

        msg_tchat_t params = {game_type + 13, player.player_id, player.team,
                              strlen(buffer), buffer};
        uint8_t *message = ms_tchat(params);
        int r =
            send(sock_client, message, sizeof(uint8_t) * 5 + strlen(buffer), 0);

        if (r == -1) {
          perror("client.c: main: send()");
          exit(EXIT_FAILURE);
        }

        printf("Envoyé: %d", r);
      }
    }

    /*msg_game_t params = {grid.game_type, player.player_id, player.team, num,
    a}; uint32_t message = ms_game(params); sendto(sock_client, &message,
    sizeof(message), 0, (struct sockaddr *)&mc.adr, sizeof(mc.adr));

    num++;*/

    // Clear la fenêtre
    // clear();

    // TODO: Recv/send msg avec le serveur
  }

  // Ferme la fenêtre
  endwin();
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
 * Configure la socket UDP pour la communication multicast.
 * @param mc La structure multicast.
 * @param adr_mdiff L'adresse de multidiffusion.
 * @param game_data Les données de la partie.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int config_udp(multicast_client_t *mc, char *adr_mdiff,
               msg_game_data_t game_data) {
  // Variables d'erreur
  int r;

  // Initialisation des adresses
  memset(&mc->r_adr, 0, sizeof(mc->r_adr));
  memset(&mc->s_adr, 0, sizeof(mc->s_adr));

  // Déclarer la socket
  mc->sock = socket(PF_INET6, SOCK_DGRAM, 0);
  if (mc->sock == -1) {
    perror("client.c: config_udp(): socket(): création socket failed");
    return -1;
  }

  // Autoriser la réutilisation de l'adresse locale
  int o = 1;
  r = setsockopt(mc->sock, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o));
  if (r < 0) {
    perror("client.c: config_udp(): setsockopt(): REUSEADDR échoué");
    return -1;
  }

  // Initialisation de l'adresse de réception
  memset(&mc->r_adr, 0, sizeof(mc->r_adr));
  mc->r_adr.sin6_family = AF_INET6;
  mc->r_adr.sin6_addr = in6addr_any;
  mc->r_adr.sin6_port = htons(game_data.port_mdiff);

  // Initialisation de l'adresse d'envoi
  memset(&mc->s_adr, 0, sizeof(mc->s_adr));
  mc->s_adr.sin6_family = AF_INET6;
  mc->s_adr.sin6_port = htons(game_data.port_udp);
  inet_pton(AF_INET6, adr_mdiff, &mc->s_adr.sin6_addr);

  // On lie la socket à l'adresse de réception (port multicast)
  r = bind(mc->sock, (struct sockaddr *)&mc->r_adr, sizeof(mc->r_adr));
  if (r < 0) {
    perror("client.c: config_udp(): bind(): bind socket au port failed");
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
    perror("client.c: config_udp(): setsockopt: abonnement échoué");
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
  nodelay(stdscr, TRUE);
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
  // IDEA: On peut transmettre cette info dans les msg TCP précédents
  int len = sizeof(uint8_t) * (6 + HEIGHT * WIDTH);
  uint8_t *msg = malloc(len);
  socklen_t len_adr = sizeof(mc.r_adr);

  // Réception des données
  int r =
      recvfrom(mc.sock, msg, len, 0, (struct sockaddr *)&mc.r_adr, &len_adr);

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

int recv_msg_grid_tmp(msg_grid_t *grid, multicast_client_t mc) {
  int len = sizeof(uint8_t) * 5 + (HEIGHT * WIDTH * 3);
  uint8_t *msg = malloc(len);
  socklen_t len_adr = sizeof(mc.r_adr);

  // Réception des données
  int r =
      recvfrom(mc.sock, msg, len, 0, (struct sockaddr *)&mc.r_adr, &len_adr);

  //  Gestions des erreurs
  if (!r || r < 0) {
    perror("client.c: recv_grid: recvfrom");
    return -1;
  }

  // On récupère les données de la grid
  msg_grid_tmp_t grid_tmp = mg_grid_tmp(msg);

  for (int i = 0; i < grid_tmp.nb_cases; i++) {
    int x = grid_tmp.grille[i * 3];
    int y = grid_tmp.grille[i * 3 + 1];
    int val = grid_tmp.grille[i * 3 + 2];
    grid->grille[y * grid->largeur + x] = val;
  }

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
