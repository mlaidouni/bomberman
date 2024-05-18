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

int main(int argc, char const *argv[]) {
  // NOTE: Le port devra être passé en argument
  const int port = 8081;

  /* ********** Connexion en mode TCP ********** */

  // Connexion en mode TCP au serveur
  int sock_client = connect_to_server(port);
  // Gestion des erreurs
  if (sock_client == -1)
    exit(EXIT_FAILURE);

  printf("Connected to server !\n");

  /* ********** Gestion du choix de partie ********** */

  // On demande au joueur le type de partie qu'il veut rejoindre
  int game_type;
  printf("Entrer 0 pour jouer à 4 joueurs, 1 pour jouer en équipes: ");
  scanf("%d", &game_type);
  join_game(sock_client, game_type);

  /* ********** Gestion de la configuration de la partie ********** */

  // On attend la réception des données de la partie
  msg_game_data_t game_data;
  recv_msg_game_data(&game_data, sock_client);

  // On convertit l'adresse de uin8_t* à char*
  char adr_mdiff[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, game_data.adr_mdiff, adr_mdiff, INET6_ADDRSTRLEN);

  affiche_data_partie(&game_data, adr_mdiff); // TODELETE

  // Abonnement à l'adresse de multicast
  multicast_client_t mc;
  if (abonnement_mdiff(&mc, adr_mdiff, game_data.port_mdiff) < 0)
    return -1; // En cas d'échec on exit, pour l'instant.

  // On s'annonce prêt à jouer au serveur
  if (ready(sock_client, game_type, game_data.player_id, 0) < 0)
    return -1; // En cas d'échec on exit, pour l'instant.

  /* ********** Gestion des messages de la partie... ********** */
  uint8_t *message = recv_grille(mc);
  msg_grid_t grid = mg_game_grid(message);

  // Affichage des données de la structure
  printf(
      "\033[35m Données de la grille reçues: HEIGHT: %d, WIDTH: %d\n\033[0m",
      grid.hauteur, grid.largeur);

  while (1)
    // TODO: Recv/send msg avec le serveur
    ;

  // Fermeture de la socket client: à la fin de la partie seulement
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

/**
 * Envoie un message de type 'join' au serveur.
 * @param sock_client La socket client.
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
 * @param sock_client La socket client.
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

/**
 * Envoie un message de type 'game' au serveur.
 */
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

/* ********** Fonctions utilitaires ********** */

/**
 * Envoie un message au serveur.
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

/**
 * Reçoit un message du serveur.
 * @param sock_client La socket client.
 * @param message Un pointeur vers le message à recevoir.
 * @param msg_size La taille du message.
 * @param type Le type de réception, i.e le nom de la fonction appelante.
 * @return Le nombre d'octets reçus ou -1 en cas d'erreur.
 */
int recv_message(int sock_client, void *message, size_t msg_size, char *type) {
  // Réception du message
  int r = recv(sock_client, message, msg_size, 0);

  // Gestion des erreurs
  if (r == -1) {
    char err_msg[100];
    sprintf(err_msg, "client.c: %s: recv message failed", type);
    perror(err_msg);
    close(sock_client);
    return -1;
  }

  return r;
}

/**
 * Réception du msg_game_data du serveur.
 * @param game_data La structure a remplir.
 * @param sock_client La socket sur laquelle on réceptionne.
 * @return 0 si tout se passe bien, -1 sinon.
 */
int recv_msg_game_data(msg_game_data_t *game_data, int sock_client) {
  uint8_t *msg = malloc(sizeof(uint8_t) * 22); // FIX: à free

  int r = recv(sock_client, msg, sizeof(uint8_t) * 22, 0);
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

uint8_t *recv_grille(multicast_client_t mc) {
  puts("\033[33mReady envoyé. Entrée dans le while(1) ...\033[0m");

  puts("\033[33m Réception de la grille...\033[0m");
  int len = sizeof(uint8_t) * (6 + HEIGHT * WIDTH);
  uint8_t *message = malloc(len);
  // Réception des données
  int bytes = recvfrom(mc.sock, message, len, 0, (struct sockaddr *)&mc.adr,
                       (socklen_t *)sizeof(mc.adr));
  if (bytes == 0) {
    puts("alo");
    exit(EXIT_FAILURE);
  }

  puts("\033[33m Réception de la grille...\033[0m");

  return message;
}
