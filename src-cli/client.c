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
    sprintf(err_msg, "src-cli/client.c: %s: send message failed", type);
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
    sprintf(err_msg, "src-cli/client.c: %s: recv message failed", type);
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
  const int port = 8080;

  // Connexion en mode TCP au serveur
  int sock_client = connect_to_server(port);
  // Gestion des erreurs
  if (sock_client == -1)
    exit(EXIT_FAILURE);

  printf("Connected to server\n");

  // Envoi du mode de jeu au serveur
  int r = join_game(sock_client, 1);
  // Gestion des erreurs
  if (r == -1) {
    puts("main: could not join game");
    exit(EXIT_FAILURE);
  }

  char ip_multicast[12];
  char port_multicast[6];

  ssize_t bytes_received = recvfrom(sock_client, ip_multicast, sizeof(ip_multicast), 0, NULL, NULL);

  // Gestion des erreurs
  if (bytes_received == -1) {
    perror("main: recvfrom()");
    close(sock_client);
    exit(EXIT_FAILURE);
  }
  printf("ip multicast: %s\n", ip_multicast);
    
  bytes_received = recvfrom(sock_client, port_multicast, sizeof(port_multicast), 0, NULL, NULL);
  
  // Gestion des erreurs
  if (bytes_received == -1) {
    perror("main: recvfrom()");
    close(sock_client);
    exit(EXIT_FAILURE);
  }
  printf("port multicast: %s\n", port_multicast);
 

  while (1) {
    // On attend la réception des données de la partie
    

    

    // On s'abonne à l'adresse de multicast (connexion UDP)
    /*
    struct sockaddr_in6 multicast_info;
    memset(&multicast_info, 0, sizeof(multicast_info));
    multicast_info.sin6_family = AF_INET6;
    multicast_info.sin6_port = htons(MULTICAST_PORT);
    inet_pton(AF_INET6, MULTICAST_ADDRESS, &multicast_info.sin6_addr);
    */
  }


    // On s'annonce prêt au serveur
    // ...
    // Autres traitements à effectuer
    // ...

    // Fermeture de la socket client: à la fin de la partie seulement
  close(sock_client);
  return 0;
}
