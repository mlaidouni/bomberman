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
  msg_join_ready params;
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
  msg_join_ready params;
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
  msg_game params;
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

// IDEA: Doit-on abstraire les fonctions suivantes ?

/**
 * Récupère les données de la partie envoyées par le serveur.
 * @param sock_client La socket client.
 * @param data Un pointeur vers la structure où stocker les données de la
 * partie.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int get_game_data(int sock_client, msg_game_data *data) {
  // Réception des données de la partie
  uint8_t *message;
  int bytes =
      recv_message(sock_client, message, sizeof(message), "get_game_data");
  // Gestion des erreurs (en partie gérée par recv_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets reçus (voir s'ils ont tous été reçus)

  // On récupère les données de la partie
  *data = mg_game_data(message);

  return 0;
}

/**
 * Récupère la grille envoyée par le serveur.
 * @param sock_client La socket client.
 * @param grid Un pointeur vers la structure où stocker la grille.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int get_grid(int sock_client, msg_grid *grid) {
  // Réception de la grille
  uint8_t *message;
  int bytes = recv_message(sock_client, message, sizeof(message), "get_grid");
  // Gestion des erreurs (en partie gérée par recv_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets reçus (voir s'ils ont tous été reçus)

  // On récupère la grille
  *grid = mg_game_grid(message);

  return 0;
}

// Réception de la grille temporaire
int get_grid_tmp(int sock_client, msg_grid_tmp *grid) {
  // Réception de la grille temporaire
  uint8_t *message;
  int bytes =
      recv_message(sock_client, message, sizeof(message), "get_grid_tmp");
  // Gestion des erreurs (en partie gérée par recv_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets reçus (voir s'ils ont tous été reçus)

  // On récupère la grille temporaire
  *grid = mg_grid_tmp(message);

  return 0;
}

/**
 * Récupère la fin de partie envoyée par le serveur.
 * @param sock_client La socket client.
 * @param end_game Un pointeur vers la structure où stocker la fin de partie.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int get_end_game(int sock_client, msg_end_game *end_game) {
  // Réception de la fin de partie
  uint16_t message;
  int bytes =
      recv_message(sock_client, &message, sizeof(message), "get_end_game");
  // Gestion des erreurs (en partie gérée par recv_message())
  if (bytes == -1)
    return -1;

  // TODO: Gestion du nb d'octets reçus (voir s'ils ont tous été reçus)

  // On récupère la fin de partie
  *end_game = mg_end_game(message);

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

  // On attend la réception des données de la partie

  // On s'abonne à l'adresse de multicast (connexion UDP)

  // On s'annonce prêt au serveur

  // ...

  // Fermeture de la socket client: à la fin de la partie seulement
  close(sock_client);
  return 0;
}
