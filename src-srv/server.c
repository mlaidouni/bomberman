#include "server.h"
#include "../lib/message.h"
#include "partie.h"

// VARIABLES
server_t srv = {0};

// Affiche les informations de connexion du client.
void affiche_connexion(struct sockaddr_in6 adrclient) {
  char adr_buf[INET6_ADDRSTRLEN];
  memset(adr_buf, 0, sizeof(adr_buf));

  inet_ntop(AF_INET6, &(adrclient.sin6_addr), adr_buf, sizeof(adr_buf));
  printf("adresse client : IP: %s port: %d\n", adr_buf,
         ntohs(adrclient.sin6_port));
}

void affiche_parties() {
  // Pour chaque partie du serveur, on affiche index type et nb de joueurs
  parties_t parties = srv.parties;

  printf("\033[35mNombre de parties: %d\033[0m\n", parties.nb_parties);

  for (int i = 0; i < parties.nb_parties; i++) {
    partie_t p = parties.parties[i];
    int nbj = p.nb_joueurs;
    printf("\033[35m-> Partie %d: type:%d, nbj:%d\n", i, p.type, nbj);

    // Pour chaque joueur de la partie, on affiche son id et son état
    for (int j = 0; j < nbj; j++)
      printf("\tJoueur %d: id:%d, ready:%d\n", j, p.joueurs[j].id,
             p.joueurs[j].ready);
    printf("\033[0m");
  }
}

int main(int argc, char **args) {
  // Tests des couleurs
  // printf("\033[31mRouge\033[0m\n");
  // printf("\033[32mVert\033[0m\n");
  // printf("\033[33mJaune\033[0m\n");
  // printf("\033[34mBleu\033[0m\n");
  // printf("\033[35mMagenta\033[0m\n");
  // printf("\033[36mCyan\033[0m\n");
  // printf("\033[37mBlanc\033[0m\n");
  // printf("\033[90mNoir brillant\033[0m\n");
  // printf("\033[91mRouge brillant\033[0m\n");
  // printf("\033[92mVert brillant\033[0m\n");
  // printf("\033[93mJaune brillant\033[0m\n");
  // printf("\033[94mBleu brillant\033[0m\n");
  // printf("\033[95mMagenta brillant\033[0m\n");
  // printf("\033[96mCyan brillant\033[0m\n");
  // printf("\033[97mBlanc brillant\033[0m\n");

  // NOTE: Le port devra être passé en argument
  const int tcp_port = 8081;

  // NOTE: Les variables du serveur sont initialisées dans cette fonction
  // Création de la connexion TCP
  if (create_TCP_connection(tcp_port) < 0)
    exit(EXIT_FAILURE);

  /* ********** Gestion des messages TCP des clients ********** */

  /* On commence par ajouter la socket server à l'ensemble des sockets à
   * surveiller */
  init_poll();

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
      if (poll_accept()) {
        perror("server.c: main(): poll_accept()");
        exit(EXIT_FAILURE); // Si ça passe mal, on exit (pour l'instant)
      }
    }

    /* ********** Gestions des sockets clients ********** */

    for (int i = 1; i < srv.nb_clients + 1; i++) {
      // Ici on gère la reception des messages join, ready et tchat des clients

      // La socket du client
      int sock_client = srv.socks[i].fd;

      // Si on reçoit un message du client
      if (srv.socks[i].revents & POLLIN) {

        // Si le client n'est dans aucune partie, c'est un message 'join'
        if (get_partie(sock_client) < 0) {
          // On gère le message 'join'
          int r = poll_join(sock_client, i);

          // Gestion des erreurs
          if (r == -1)
            // Si recv échoue (renvoie -1 ou 0), on passe à la socket suivante.
            break;
          else if (r == -2)
            // Si init_partie ou send_game_data échoue, on exit (pour l'instant)
            exit(EXIT_FAILURE);
        }

        // Si le client est dans une partie, c'est un message 'ready'
        /* TODO: (plus tard on devra regarder les 2 premiers octets pour savoir
         * si c'est ready ou tchat) */
        else {
          // On gère le message 'ready'
          int partie_index = poll_ready(sock_client);

          // Gestion des erreurs
          if (partie_index == -1)
            // Si recv échoue (renvoie -1 ou 0), on passe à la socket suivante.
            break;
          else if (partie_index == -2)
            // Si init_partie ou send_game_data échoue, on exit (pour l'instant)
            exit(EXIT_FAILURE);

          // On vérifie si la partie est prête à être lancée
          if (is_partie_ready(partie_index)) {
            // TODO: Gestion des différentes parties (threads, ...)
            printf(
                "server.c: main(): poll socks: partie %d prête à être lancée\n",
                partie_index);
            // TODO: On lance la partie
            start_game(&srv.parties.parties[partie_index]);
          }
        }
      }
    }

    // Fin de la gestion des sockets clients...
  }

  // On free toutes les ressources allouées côté server.c et partie.c
  for (int i = 0; i < srv.parties.nb_parties; i++)
    free_partie(&srv.parties.parties[i]);
  free(srv.parties.parties);
  free(srv.socks);
  free(srv.clients);

  // On ferme la socket serveur
  close(srv.tcp_sock);
  return 0;
}

/* ********** Fonctions server ********** */

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

      partie_t partie = srv.parties.parties[get_partie(sock_client)];
      partie.nb_joueurs--;
      break;
    }
  }

  // On retire le client de la partie
  partie_t *partie = &srv.parties.parties[get_partie(sock_client)];

  // On supprime le joueur de la partie
  for (int j = 0; j < partie->nb_joueurs; j++) {
    if (partie->joueurs[j].client.sock == sock_client) {
      // On décale les joueurs suivants
      for (int k = j; k < partie->nb_joueurs - 1; k++)
        partie->joueurs[k] = partie->joueurs[k + 1];

      // On réalloue la mémoire
      partie->joueurs =
          realloc(partie->joueurs, (partie->nb_joueurs - 1) * sizeof(joueur_t));

      // On décrémente le nombre de joueurs
      partie->nb_joueurs--;

      break;
    }
  }

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

/**
 * Vérifie si une partie est prête à être lancée, i.e si elle a 4 joueurs prêts.
 * @param partie_index L'index de la partie à vérifier.
 * @return 1 si la partie est prête, 0 sinon.
 */
int is_partie_ready(int partie_index) {
  // On récupère la partie
  partie_t partie = srv.parties.parties[partie_index];

  if (partie.nb_joueurs != 4)
    return 0;

  // On vérifie que tous les joueurs sont prêts
  int ready = 1;
  for (int j = 0; j < partie.nb_joueurs; j++) {
    if (!partie.joueurs[j].ready) {
      ready = 0;
      break;
    }
  }

  return ready;
}

//  Initialise le poll en ajoutant la socket serveur.
void init_poll() {
  srv.socks = malloc(sizeof(struct pollfd));
  // La socket serveur sera toujours identifiée par l'indice 0
  srv.socks[0].fd = srv.tcp_sock;
  // On la surveille en lecture
  srv.socks[0].events = POLLIN;
}

/**
 * Gère les demandes de connexion TCP des clients au serveur.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int poll_accept() {
  // Le client qui va se connecter
  client_t client = {0};

  // On essaye d'accepter le client
  if (!accept_client(&client)) {
    // Si le client est accepté, on réalloue la mémoire
    srv.socks =
        realloc(srv.socks, (srv.nb_clients + 1) * sizeof(struct pollfd));

    // Si la réallocation a échoué
    if (srv.socks == NULL) {
      perror("server.c: poll_accept(): realloc()");
      return -1;
    }

    // Ajout de la socket client dans le tableau des sockets
    srv.socks[srv.nb_clients].fd = client.sock;
    srv.socks[srv.nb_clients].events = POLLIN;
  }

  // NOTE: On ne fait rien si le client n'est pas accepté

  return 0;
}

/**
 * Gère les messages 'join' des clients.
 * @param sock_client La socket du client.
 * @param sock_index L'index du client dans la liste des sockets à surveiller.
 * @return 0 si tout s'est bien passé, -1 si le recv a échoué, -2 sinon.
 */
int poll_join(int sock_client, int sock_index) {
  // On reçoit le message
  uint16_t message;
  // FIXME: boucler sur le recv pour être sûr de tout recevoir
  int bytes = recv(sock_client, &message, sizeof(message), 0);

  // Gestion des erreurs
  if (bytes < 0) {
    // On déconnecte le client
    deconnect_client(sock_client);
    perror("server.c: poll_join: recv()");
    return -1; // Si ça se passe mal, on ira a la socket suivante
  }
  // Si le client s'est déconnecté
  if (!bytes) {
    puts("server.c: poll_join: client déconnecté !");
    // On déconnecte le client
    deconnect_client(sock_client);
    return -1; // Si ça se passe mal, on ira a la socket suivante
  }

  // Sinon, on décode le message
  msg_join_ready_t params = mg_join(message);

  // On gère la création ou l'ajout du joueur à une partie
  if (init_partie(params, srv.clients[sock_index - 1])) {
    puts("server.c: poll_join: init_partie()");
    return -2; // Si ça se passe mal, on exit dans main() (pour l'instant)
  }

  // On envoie les données de la partie au client
  if (send_game_data(sock_client)) {
    puts("server.c: poll_join: send_game_data()");
    return -2; // Si ça se passe mal, on exit dans main() (pour l'instant)
  }

  return 0;
}

/**
 * Gère les messages 'ready' des clients, en mettant à jour le statut du joueur.
 * @param sock_client La socket du client.
 * @return L'index de la partie dans la liste des parties si tout s'est bien
 * passé, -1 si le recv a échoué, -2 sinon.
 */
int poll_ready(int sock_client) {
  // On reçoit le message
  uint16_t message;
  // FIXME: boucler sur le recv pour être sûr de tout recevoir
  int bytes = recv(sock_client, &message, sizeof(message), 0);

  // Gestion des erreurs
  if (bytes < 0) {
    // On déconnecte le client
    deconnect_client(sock_client);
    perror("server.c: poll_ready: recv()");
    return -1; // Si ça se passe mal, on ira a la socket suivante
  }
  // Si le client s'est déconnecté
  if (!bytes) {
    puts("server.c: poll_ready: client déconnecté !");
    // On déconnecte le client
    deconnect_client(sock_client);
    return -1; // Si ça se passe mal, on ira a la socket suivante
  }

  // Sinon, on décode le message
  /* TODO: Utiliser ça pour récupérer le joueur avec partie.joueurs[params.id]
   * (quelques lignes plus loin) */
  msg_join_ready_t params = mg_ready(message);

  // TODELETE: Affichage pour éviter l'inutilisation de params
  printf("server.c: poll_ready: type de partie: %d\n", params.game_type);

  // On récupère la partie dans laquelle le joueur est
  int partie_index = get_partie(sock_client);

  // Gestion des erreurs
  if (partie_index < 0) {
    perror("server.c: poll_ready: get_partie()");
    return -2;
  }

  partie_t *partie = &srv.parties.parties[partie_index];

  // On récupère le joueur et on le met à jour
  joueur_t *joueur = get_joueur(partie, sock_client);

  // Gestion des erreurs
  if (joueur == NULL) {
    perror("server.c: poll_ready: get_joueur()");
    return -2;
  }

  joueur->ready = 1;

  return partie_index;
}

int poll_tchat(int sock_client) {
  uint8_t *message;
  int r = 0;
  while (r < 3) {
    r += recv(sock_client, message + r, 3 - r, 0);
  }
  int len = message[2];
  r = 0;
  while (r < len) {
    r += recv(sock_client, message + 2 + r, len - r, 0);
  }
  printf("server.c: poll_tchat: message reçu: %s\n", message + 3);
  partie_t *partie = &srv.parties.parties[get_partie(sock_client)];
  for (int i = 0; i < partie->nb_joueurs; i++) {
    send(partie->joueurs[i].client.sock, message, 3 + len, 0);
  }
  return 0;
}

/* ********** Fonctions utilitaires ********** */

/**
 * Initialise une structure msg_game_data_t avec les données de la partie.
 * @param partie La partie dont on veut récupérer les données.
 * @param game_data La structure msg_game_data_t à initialiser.
 */
void init_msg_game_data(partie_t partie, msg_game_data_t *game_data) {
  uint8_t buf[16];

  // On convertit l'adresse multicast en uint8_t*
  int pton_result = inet_pton(AF_INET6, partie.adr_mdiff, buf);
  if (pton_result <= 0) {
    if (pton_result == 0)
      printf("server.c: init_msg_game_data: inet_pton: adresse invalide !\n");
    else
      perror("server.c: init_msg_game_data: inet_pton");
    exit(EXIT_FAILURE); // Pour l'instant, on exit si ça se passe mal
  }

  // On remplit la structure msg_game_data_t
  memcpy(&game_data->adr_mdiff, buf, sizeof(buf));
  game_data->port_mdiff = partie.port_mdiff;
  game_data->port_udp = partie.port_udp;
  game_data->game_type = partie.type;
  joueur_t added_player = partie.joueurs[partie.nb_joueurs - 1];
  game_data->player_id = added_player.id;
  game_data->team_id = added_player.team;
}

/**
 * Envoie les données de la partie à un client qui vient d'être ajouté.
 * @param sock_client La socket du client à qui envoyer les données.
 * @return 0 si tout s'est bien passé, -1 sinon.
 */
int send_game_data(int sock_client) {
  // On récupère la partie dans laquelle le joueur a été ajouté
  int partie_index = get_partie(sock_client);
  partie_t partie = srv.parties.parties[partie_index];

  // On récupère les données de la partie
  msg_game_data_t game_data;
  init_msg_game_data(partie, &game_data);

  // On convertit ces données en message
  uint8_t *msg = ms_game_data(game_data);

  // On envoie le message
  int bytes = 0;
  while (bytes < 22) { // FIXME: magic number
    int sent = send(sock_client, msg + bytes, 22 - bytes, 0);

    // Gestion des erreurs
    if (sent < 0) {
      perror("server.c: main(): poll socks: send()");
      return -1; // Si ça passe mal, on exit (pour l'instant)
    }

    bytes += sent;
  }

  return 0;
}

// Récupérer l'indice du client à partir de la socket
int get_client(int sock_client) {
  for (int i = 0; i < srv.nb_clients; i++) {
    if (srv.clients[i].sock == sock_client)
      return i;
  }
  return -1;
}

/**
 * Récupére l'indice de la partie à partir de la socket, si elle existe.
 * @param sock_client La socket du client.
 * @return L'index de la partie dans laquelle le client est, -1 sinon.
 */
int get_partie(int sock_client) {
  for (int i = 0; i < srv.parties.nb_parties; i++) {
    for (int j = 0; j < srv.parties.parties[i].nb_joueurs; j++) {
      if (srv.parties.parties[i].joueurs[j].client.sock == sock_client)
        return i;
    }
  }
  return -1;
}

// Récupérer le joueur à partir de la socket
joueur_t *get_joueur(partie_t *partie, int sock_client) {
  for (int i = 0; i < partie->nb_joueurs; i++) {
    if (partie->joueurs[i].client.sock == sock_client)
      return &partie->joueurs[i];
  }
  return NULL;
}
