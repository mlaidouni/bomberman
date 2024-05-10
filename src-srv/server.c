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

  /* ********** Gestion des messages TCP des clients ********** */

  /* On commence par ajouter la socket server à l'ensemble des sockets à
   * surveiller */
  srv.socks = malloc(sizeof(struct pollfd));
  // La socket serveur sera toujours identifiée par l'indice 0
  srv.socks[0].fd = srv.tcp_sock;
  // On la surveille en lecture
  srv.socks[0].events = POLLIN;

  while (1) {
    affiche_parties(); // TODELETE: On affiche les parties

    /**
     * srv.socks: l'ensemble des sockets à surveiller
     * srv.nb_clients + 1: le nombre de sockets à surveiller
     * timeout = -1: donc le poll va bloquer indéfiniment
     */
    // On bloque ici jusqu'à ce qu'une socket soit prête
    poll(srv.socks, srv.nb_clients + 1, -1);

    /* Si la socket du serveur est en activité en lecture, cela signifie qu'un
     * client tente de se connecter */
    if (srv.socks[0].revents & POLLIN) {
      // Le client qui va se connecter
      client_t client = {0};

      // On essaye d'accepter le client
      if (!accept_client(&client)) {
        // Si le client est accepté, on réalloue la mémoire
        srv.socks =
            realloc(srv.socks, (srv.nb_clients + 1) * sizeof(struct pollfd));

        // Ajout de la socket client dans le tableau des sockets
        srv.socks[srv.nb_clients].fd = client.sock;
        srv.socks[srv.nb_clients].events = POLLIN;
      }

      // NOTE: On ne fait rien si le client n'est pas accepté
    }

    /* ********** Gestions des sockets clients ********** */
    for (int i = 1; i < srv.nb_clients + 1; i++) {
      // Ici on gère la reception des messages join, ready et tchat des clients

      // La socket du client
      int sock_client = srv.socks[i].fd;

      // Si on reçoit un message du client
      if (srv.socks[i].revents & POLLIN) {

        // Si le client n'est dans aucune partie, c'est un message 'join'
        if (is_client_in_partie(sock_client)) {
          printf("\033[30m server.c: main: poll socks: demande JOIN\033[0m \n");

          // On reçoit le message
          uint16_t message;
          int bytes = recv(sock_client, &message, sizeof(message), 0);

          // Gestion des erreurs
          if (bytes < 0) {
            // On déconnecte le client
            deconnect_client(sock_client);
            perror("server.c: main(): poll socks: recv()");
            break;
          }
          // Si le client s'est déconnecté
          if (!bytes) {
            puts("server.c: main(): poll socks: client déconnecté !");
            // On déconnecte le client
            deconnect_client(sock_client);
            break;
          }

          // Sinon, on décode le message
          msg_join_ready_t params = mg_join(message);

          // On gère la création ou l'ajout du joueur à une partie
          if (init_partie(params, srv.clients[i - 1])) {
            puts("server.c: main(): poll socks: init_partie()");
            exit(EXIT_FAILURE); // Si ça passe mal, on exit (pour l'instant)
          }
        }

        // Si le client est dans une partie, c'est un message 'ready'
        /* TODO: (plus tard on devra regarder les 2 premiers octets pour savoir
         * si c'est ready ou tchat) */
        else {
          // On reçoit le message
          uint16_t message;
          int bytes = recv(sock_client, &message, sizeof(message), 0);

          // Gestion des erreurs
          if (bytes < 0) {
            // On déconnecte le client
            deconnect_client(sock_client);
            perror("server.c: main(): poll socks: recv()");
            break;
          }
          // Si le client s'est déconnecté
          if (!bytes) {
            puts("server.c: main(): poll socks: client déconnecté !");
            // On déconnecte le client
            deconnect_client(sock_client);
            break;
          }

          // Sinon, on décode le message
          // msg_join_ready_t params = mg_ready(message);

          // TODO: On gère le ready du joueur
          printf("\033[32mserver.c: main(): poll socks: msg_ready()\033[0m \n");
        }
      }
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

  /* Le numéro de port peut être réutilisé immédiatement après la fermeture du
   serveur */
  int o = 1;
  int r = setsockopt(sock_srv, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o));
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: create_TCP_connection(): setsockopt()");
    return -1;
  }

  // On lie la socket au port
  r = bind(sock_srv, (struct sockaddr *)&addr, sizeof(addr));
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: create_TCP_connection(): bind()");
    return -1;
  }

  // Le serveur est prêt à écouter les connexions sur le port
  r = listen(sock_srv, 0);
  // Gestions des erreurs
  if (r < 0) {
    perror("server.c: create_TCP_connection(): listen()");
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
    perror("server.c: accept_client(): accept()");
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

/**
 * Déconnecte un client du serveur.
 * @param sock_client La socket du client à déconnecter.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int deconnect_client(int sock_client) {
  // On supprime le client de la liste des clients
  for (int j = 0; j < srv.nb_clients; j++) {
    if (srv.clients[j].sock == sock_client) {
      // On ferme la socket du client
      close(sock_client);

      // On décale les clients suivants
      for (int k = j; k < srv.nb_clients - 1; k++)
        srv.clients[k] = srv.clients[k + 1];

      // On réalloue la mémoire
      srv.clients =
          realloc(srv.clients, (srv.nb_clients - 1) * sizeof(client_t));

      // On décrémente le nombre de clients
      srv.nb_clients--;

      break;
    }
  }

  // TODO: Retirer le client de la partie

  // On supprime la socket du client du tableau des sockets
  for (int i = 0; i < srv.nb_clients + 1; i++) {
    if (srv.socks[i].fd == sock_client) {
      // On ferme la socket du client
      close(sock_client);

      // On décale les sockets suivantes
      for (int k = i; k < srv.nb_clients + 1; k++)
        srv.socks[k] = srv.socks[k + 1];

      // On réalloue la mémoire
      srv.socks =
          realloc(srv.socks, (srv.nb_clients + 1) * sizeof(struct pollfd));

      break;
    }
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

// Renvoie -1 si la socket client n'est dans aucune partie, 0 sinon
int is_client_in_partie(int sock_client) {
  for (int i = 0; i < srv.parties.nb_parties; i++) {
    for (int j = 0; j < srv.parties.parties[i].nb_joueurs; j++) {
      if (srv.parties.parties[i].joueurs[j].client.sock == sock_client)
        return 0;
    }
  }
  return -1;
}
