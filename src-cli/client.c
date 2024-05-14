#include "client.h"

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

int main(int argc, char const *argv[]) {
  // NOTE: Le port devra être passé en argument
  const int port = 8081;

  // Connexion en mode TCP au serveur
  int sock_client = connect_to_server(port);
  // Gestion des erreurs
  if (sock_client == -1)
    exit(EXIT_FAILURE);

  printf("Connected to server\n");

  // Si on entre 0, on veut jouer à 4 joueurs, si on entre 1 on veut jouer à 2
  // équipes, envoyer le chiffre entré au serveur
  // Demander à l'utilisateur d'entrer 0 ou 1
  int game_type;
  printf("Entrer 0 pour jouer à 4 joueurs, 1 pour jouer en équipes: ");
  scanf("%d", &game_type);
  join_game(sock_client, game_type);

  // while (1) {
  // TODO: On attend la réception des données de la partie
  uint8_t *msg = malloc(sizeof(uint8_t) * 22);

  int r = recv(sock_client, msg, sizeof(uint8_t) * 22, 0);
  if (r == -1) {
    perror("client.c: main: recv");
    return -1;
  }

  // On récupère les données de la partie
  msg_game_data_t game_data = mg_game_data(msg);

  // TODELETE: START
  // On convertit l'adresse de uin8_t* à char* pour pouvoir l'afficher
  char adr_str[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &game_data.adr_mdiff, adr_str, INET6_ADDRSTRLEN);

  printf(
      "\033[35m Données de la partie reçues:\n\t Adresse multicast: %s \n\t "
      "adr_mdiff: %d \n\t port_mdiff: %d \n\t "
      "port_udp:%d \n\t game_type: %d \n\t player_id: %d \n\t team_id: "
      "%d\n\033[0m",
      adr_str, *(game_data.adr_mdiff), game_data.port_mdiff, game_data.port_udp,
      game_data.game_type, game_data.player_id, game_data.team_id);

  // TODELETE: END

  // TODO: On s'abonne à l'adresse de multicast (connexion UDP)
  // }

  // TODO: On s'annonce prêt au serveur
  // TODELETE: DEBUT
  printf("Entrer player_id, i.e l'odre d'ajout du client (0 à 3): ");
  int player_id;
  scanf("%d", &player_id);

  // On est prêt à jouer
  ready(sock_client, game_type, player_id, 0);

  puts("\033[33mReady envoyé. Entrée dans le while(1) ...\033[0m");
  while (1)
    ;
  // TODELETE: FIN

  multicast_client_t mc;
  memset(&mc.adr, 0, sizeof(mc.adr));
  mc.adr.sin6_family = AF_INET6;
  mc.adr.sin6_port = htons(game_data.port_mdiff);
  memcpy(&mc.adr, &game_data.adr_mdiff, sizeof(game_data.adr_mdiff));

  mc.sock = socket(PF_INET6, SOCK_DGRAM, 0);
  if (mc.sock == -1) {
    perror("client.c: main: socket failed");
    return -1;
  }

  if (bind(mc.sock, (struct sockaddr *)&mc.adr, sizeof(mc.adr)) < 0) {
    perror("client.c: main: bind failed");
    close(mc.sock);
    return -1;
  }

  while (1) {

    // On s'abonne à l'adresse de multicast (connexion UDP)

    // On s'annonce prêt au serveur

    // recvfrom pour recevoir les messages multicast
    uint8_t *msg;
    recvfrom(mc.sock, &msg, sizeof(msg), 0, NULL, NULL);
    msg_grid_t grille = mg_game_grid(msg);
    printf("Grille reçue: %d, largeur: %d et longueur: %d\n", grille.num,
           grille.largeur, grille.hauteur);

    // On attend les messages du serveur

    // Autres traitements à effectuer
  }
  // Fermeture de la socket client: à la fin de la partie seulement
  close(sock_client);
  return 0;
}
