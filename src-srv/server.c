#include "server.h"
#include "../lib/message.h"
#include "partie.h"

// DEFINES

// VARIABLES
server_t srv = {0};

void affiche_parties() {
  // Pour chaque partie du serveur, on affiche index type et nb de joueurs
  for (int i = 0; i < srv.parties.nb_parties; i++) {
    printf("\033[33m server.c: while(1): Partie %d: %d, %d \033[0m \n", i,
           srv.parties.parties[i].type, srv.parties.parties[i].nb_joueurs);
  }
}

int main(int argc, char **args) {
  // NOTE: Le port devra être passé en argument
  const int tcp_port = 8081;

  // NOTE: Les variables du serveur sont initialisées dans cette fonction
  // Création de la connexion TCP
  if (create_TCP_connection(tcp_port) < 0)
    exit(EXIT_FAILURE);

  while (1) {

    affiche_parties(); // CACA: On affiche les parties

    // On accepte un client
    client_t client = {0};
    if (accept_client(&client) == 0) {
      // Si le client est accepté, on récupère le message du client
      uint16_t message;
      recv(client.sock, &message, sizeof(message), 0);

      // On décode son message
      msg_join_ready_t params = mg_join(message);

      // On gère la création ou l'ajout du joueur à une partie
      if (init_partie(params, client))
        exit(EXIT_FAILURE); // Si ça passe mal, on exit
    }
  }

  close(srv.tcp_sock);
  return 0;
}

/* ********** Fonctions server ********** */

// Affiche les informations de connexion du client.
void affiche_connexion(struct sockaddr_in6 adrclient) {
  char adr_buf[INET6_ADDRSTRLEN];
  memset(adr_buf, 0, sizeof(adr_buf));

  inet_ntop(AF_INET6, &(adrclient.sin6_addr), adr_buf, sizeof(adr_buf));
  printf("adresse client : IP: %s port: %d\n", adr_buf,
         ntohs(adrclient.sin6_port));
}

/**
 * Crée une connexion TCP sur le port donné.
 * @param port Le port sur lequel le serveur doit écouter.
 * @param srv La structure server_t où stocker les informations de connexion.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int create_TCP_connection(int port) {
  // Création de la socket serveur
  int sock_srv = socket(PF_INET6, SOCK_STREAM, 0);
  if (sock_srv < 0) {
    perror("server.c: socket(): création de la socket échouée");
    return -1;
  }

  // Création de l'adresse du destinataire (serveur)
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;

  // On lie la socket au port
  int r = bind(sock_srv, (struct sockaddr *)&addr, sizeof(addr));
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: bind(): bind échoué");
    return -1;
  }

  // Le serveur est prêt à écouter les connexions sur le port
  r = listen(sock_srv, 0);
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: listen(): listen échoué");
    return -1;
  }

  // Si tout s'est bien passé, on stocke les informations dans le serveur
  srv.tcp_sock = sock_srv;
  srv.tcp_port = port;
  srv.adr = addr;

  return 0;
}

/**
 * Accepte un client sur la socket TCP du serveur et met à jour la liste des
 * clients.
 * @param client La structure client_t où stocker les informations du client.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int accept_client(client_t *client) {
  /* Le serveur accepte une connexion et crée la socket de communication avec le
   * client */
  memset(&client->adr, 0, sizeof(client->adr));
  client->size = sizeof(client->adr);

  // TODO: Gérer le fait que accept() est bloquant

  // On accepte la connexion
  int sock_client =
      accept(srv.tcp_sock, (struct sockaddr *)&client->adr, &client->size);

  // Gestions des erreurs
  if (sock_client == -1) {
    perror("server.c: accept(): problème avec la socket client");
    return -1;
  }

  // Si tout s'est bien passé, on stocke la socket du client
  client->sock = sock_client;

  // Si la liste des clients n'est pas allouée, on l'alloue
  if (srv.nb_clients == 0)
    srv.clients = malloc(sizeof(client_t));
  else {
    // Si la liste est déjà allouée, on réalloue la mémoire
    client_t *tmp =
        realloc(srv.clients, (srv.nb_clients + 1) * sizeof(client_t));

    srv.clients = tmp;
  }

  // On affiche les informations de connexion
  affiche_connexion(client->adr);

  // On ajoute le client à la liste des clients
  srv.clients[srv.nb_clients] = *client;

  // On incrémente le nombre de clients
  srv.nb_clients++;

  return 0;
}

int gestion_connexion_tcp() {
  // On accepte un client
  client_t client = {0};
  if (accept_client(&client) == 0) {
    // Si le client est accepté, on récupère le message du client
    uint8_t message;
    recv(client.sock, &message, sizeof(message), 0);

    // On décode son message
    msg_join_ready_t params = mg_join(message);

    // On gère la création ou l'ajout du joueur à une partie
    if (init_partie(params, client))
      return -1; // Si ça passe mal, on retourne -1
  }

  return 0;
}

/* ********** Fonctions utilitaires ********** */

/**
 * Initialise une structure msg_game_data_t avec les données de la partie.
 * @param partie La partie dont on veut récupérer les données.
 * @param game_data La structure msg_game_data_t à initialiser.
 */
void init_msg_game_data(partie_t partie, msg_game_data_t game_data) {
  uint8_t buf[16];
  inet_pton(AF_INET6, partie.adr_mdiff, &buf);
  memcpy(&game_data.adr_mdiff, buf, sizeof(buf));
  game_data.port_mdiff = partie.port_mdiff;
  game_data.port_udp = partie.port_udp;
  game_data.game_type = partie.type;
  joueur_t added_player = partie.joueurs[partie.nb_joueurs - 1];
  game_data.player_id = added_player.id;
  game_data.team_id = added_player.team;
}
